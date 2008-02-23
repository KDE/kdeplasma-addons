#ifndef FAVORITEAPPLICATIONS_H_
#define FAVORITEAPPLICATIONS_H_

#include "BaseModel.h"

namespace Lancelot {
namespace Models {

class FavoriteApplications : public BaseModel {
    Q_OBJECT
public:
    FavoriteApplications();
    virtual ~FavoriteApplications();

protected:
    void load();

};

}
}

#endif /*FAVORITEAPPLICATIONS_H_*/
