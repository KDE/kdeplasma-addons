var tabIDs = new Array(); //FIXME: Would be much nicer to have it as a property of tabBar

function isEmpty(id)
{
    for (i in lists.data["List:"+id]) {
        if (i != "smart" && i != "filter" && i != "id" && i != "name")
            return false
    }
    return true
}