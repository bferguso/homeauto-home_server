var toggleVisibility = function(objectId)
{
    var obj = $("#"+objectId);
    if (obj.is(":visible"))
    {
        obj.hide();
    }
    else
    {
        obj.show();
    }
}

var getPointStyle = function(location)
{
    if (location === 'office')
    {
        return 'circle'
    }
    else if (location === 'bedroom')
    {
        return 'triangle';
    }
    return 'rectangle';
}