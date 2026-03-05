// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

use easy_color::{Hex, IntoHSV};
use qmk_via_api::{api::KeyboardApi, scan::scan_keyboards};
use std::{collections::HashMap, error::Error, future::pending, time::Duration};
use zbus::{Connection, connection, interface, message::Header};
use zbus_polkit::policykit1::{AuthorityProxy, CheckAuthorizationFlags, Subject};
use event_listener::{Event, Listener};

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

struct Helper {}

#[interface(name = "org.kde.kameleon.qmk.helper")]
impl Helper {
    async fn apply_color(
        &mut self,
        color: &str,
        #[zbus(header)] header: Header<'_>,
    ) -> zbus::fdo::Result<()> {
        is_authorized(HashMap::from([("color", color)]), header).await?;

        let hex: Hex = color.try_into().unwrap();
        let hsv = hex.to_hsv();
        let hue = hsv.hue() as f32 / 360.0 * 255.0;
        let saturation = hsv.saturation() as f32 / 100.0 * 255.0;
        let brightness = hsv.value() as f32 / 100.0 * 255.0;

        let devices = scan_keyboards()
            .map_err(|e| zbus::fdo::Error::Failed(format!("Failed to scan keyboards: {e}")))?;
        for device in devices {
            let api = KeyboardApi::from_device(&device).map_err(|e| {
                zbus::fdo::Error::Failed(format!(
                    "Failed to create API for device {:?}: {e}",
                    device.product
                ))
            })?;
            api.set_rgb_matrix_color(hue as u8, saturation as u8)
                .map_err(|e| {
                    zbus::fdo::Error::Failed(format!(
                        "Failed to set color for device {:?}: {e}",
                        device.product
                    ))
                })?;
            api.set_rgb_matrix_brightness(brightness as u8)
                .map_err(|e| {
                    zbus::fdo::Error::Failed(format!(
                        "Failed to set brightness for device {:?}: {e}",
                        device.product
                    ))
                })?;
        }

        Ok(())
    }

    async fn has_devices(&mut self, #[zbus(header)] header: Header<'_>) -> zbus::fdo::Result<bool> {
        is_authorized(HashMap::from([]), header).await?;

        let devices = scan_keyboards()
            .map_err(|i| zbus::fdo::Error::Failed(format!("Failed to scan keyboards: {i}")))?;
        Ok(devices.len() > 0)
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
