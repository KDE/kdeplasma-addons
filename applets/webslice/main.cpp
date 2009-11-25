#include <qapplication.h>
#include <qurl.h>
#include "kwebslice.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    KWebSlice slice;
    slice.setElement( QString("#hotspot") );
    slice.setLoadingText("<h1>Loading ...<h1>");
    slice.setUrl( QUrl(QString("http://www.kde.org/")) );
    slice.show();

    return app.exec();
}
