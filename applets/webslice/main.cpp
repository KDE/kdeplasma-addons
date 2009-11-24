#include <qapplication.h>
#include <qurl.h>
#include "kwebslice.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    KWebSlice slice;
    slice.setElement( QString("#block-user-0") );
    //slice.setElement( QString("#box") );
    slice.setLoadingText("<h1>Loading ...<h1>");
    slice.setUrl( QUrl(QString("http://dot.kde.org/")) );
    //slice.setUrl( QUrl("file:///home/sebas/kdesvn/install/share/apps/silk-webapp/test/testsite/beach.html") );
    slice.show();

    return app.exec();
}
