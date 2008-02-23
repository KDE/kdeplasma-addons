#include "FavoriteApplications.h"

#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

namespace Lancelot {
namespace Models {

FavoriteApplications::FavoriteApplications()
{
    load();
}

FavoriteApplications::~FavoriteApplications()
{
}

void FavoriteApplications::load()
{
    KConfig cfg(KStandardDirs::locate("config", "kickoffrc"));
    KConfigGroup favoritesGroup = cfg.group("Favorites");
 
    QList<QString> favoriteList = favoritesGroup.readEntry("FavoriteURLs", QList<QString>());
    foreach(QString favorite, favoriteList) {
        addUrl(favorite);
    }
}


}
}
