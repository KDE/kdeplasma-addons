var tabIDs = new Array(); //FIXME: Would be much nicer to have it as a property of tabBar

function isCurrentList(id)
{
    return tabIDs[tabBar.currentIndex] == id;
}