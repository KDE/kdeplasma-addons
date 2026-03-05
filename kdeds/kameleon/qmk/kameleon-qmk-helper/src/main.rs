// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

use easy_color::{Hex, IntoHSV};
use event_listener::Listener;
use qmk_via_api::{api::KeyboardApi, scan::scan_keyboards};
use std::{collections::HashMap, error::Error, time::Duration};
use zbus::{Connection, connection, interface, message::Header};
use zbus_polkit::policykit1::{AuthorityProxy, CheckAuthorizationFlags, Subject};

async fn is_authorized(
    details: HashMap<&str, &str>,
    header: Header<'_>,
) -> zbus::fdo::Result<bool> {
    let authority = AuthorityProxy::new(&Connection::system().await?).await?;
    let subject =
        Subject::new_for_message_header(&header).expect("Failed to create polkit subject");
    let result = authority
        .check_authorization(
            &subject,
            &format!("org.kde.kameleon.qmk.helper.{}", header.member().unwrap()),
            &details,
            CheckAuthorizationFlags::AllowUserInteraction.into(),
            "",
        )
        .await?;

    if !result.is_authorized {
        return Err(zbus::fdo::Error::AuthFailed(
            "Authorization failed".to_string(),
        ));
    }

    Ok(result.is_authorized)
}

// Do not put state in here, the calls should be stateless
struct Helper {}

#[interface(name = "org.kde.kameleon.qmk.helper")]
impl Helper {
    async fn apply_color(
        &mut self,
        color: &str,
        #[zbus(header)] header: Header<'_>,
    ) -> zbus::fdo::Result<()> {
        is_authorized(HashMap::from([("color", color)]), header).await?;

        let hex: Hex = color.try_into().map_err(|e| {
            zbus::fdo::Error::Failed(format!("Invalid color format (should be #RRGGBB)"))
        })?;
        let hsv = hex.to_hsv();
        let hue = (hsv.hue() as f32 / 360.0 * 255.0) as u8;
        let saturation = (hsv.saturation() as f32 / 100.0 * 255.0) as u8;
        let brightness = (hsv.value() as f32 / 100.0 * 255.0) as u8;

        let devices = scan_keyboards()
            .map_err(|e| zbus::fdo::Error::Failed(format!("Failed to scan keyboards: {e}")))?;
        for device in devices {
            let api = KeyboardApi::from_device(&device).map_err(|e| {
                zbus::fdo::Error::Failed(format!(
                    "Failed to create API for device {:?}: {e}",
                    device.product
                ))
            })?;

            // Try setting color via matrix
            if api
                .get_rgb_matrix_effect()
                .ok()
                .map(|e| e > 0)
                .unwrap_or(false)
            {
                api.set_rgb_matrix_color(hue as u8, saturation as u8)
                    .map_err(|e| {
                        zbus::fdo::Error::Failed(format!(
                            "Failed to set matrix color for device {:?}: {e}",
                            device.product
                        ))
                    })?;
                api.set_rgb_matrix_brightness(brightness as u8)
                    .map_err(|e| {
                        zbus::fdo::Error::Failed(format!(
                            "Failed to set matrix brightness for device {:?}: {e}",
                            device.product
                        ))
                    })?;
                continue;
            }

            // Fallback to rgblight if matrix is not supported
            if api
                .get_rgblight_effect()
                .ok()
                .map(|e| e > 0)
                .unwrap_or(false)
            {
                api.set_rgblight_color(hue, saturation).map_err(|e| {
                    zbus::fdo::Error::Failed(format!(
                        "Failed to set rgblight color for device {:?}: {e}",
                        device.product
                    ))
                })?;
                api.set_rgblight_brightness(brightness).map_err(|e| {
                    zbus::fdo::Error::Failed(format!(
                        "Failed to set rgblight brightness for device {:?}: {e}",
                        device.product
                    ))
                })?;
                continue;
            }

            // Else we are out of luck
            return Err(zbus::fdo::Error::Failed(format!(
                "Failed to set any color or brightness for device {:?}",
                device.product
            )));
        }

        Ok(())
    }

    async fn has_devices(&mut self, #[zbus(header)] header: Header<'_>) -> zbus::fdo::Result<bool> {
        is_authorized(HashMap::from([]), header).await?;

        let any = scan_keyboards()
            .map_err(|i| zbus::fdo::Error::Failed(format!("Failed to scan keyboards: {i}")))?
            .iter()
            .any(|device| {
                let api =
                    KeyboardApi::from_device(&device).expect("Failed to create API for device");
                api.get_rgblight_effect().is_ok() || api.get_rgb_matrix_effect().is_ok()
            });
        Ok(any)
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let helper = Helper {};
    let conn = connection::Builder::system()?
        .name("org.kde.kameleon.qmk.helper")?
        .serve_at("/org/kde/kameleon/qmk/helper", helper)?
        .build()
        .await?;

    let mut listener = conn.monitor_activity();
    while listener.wait_timeout(Duration::from_secs(60)).is_some() {
        listener = conn.monitor_activity();
    }

    Ok(())
}
