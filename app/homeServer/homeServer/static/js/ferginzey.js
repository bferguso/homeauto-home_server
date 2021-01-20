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

var getLocationColour = function (location)
{
    if (location === 'office')
    {
        return 'rgba(234, 47, 3, 0.6)'
    }
    else if (location === 'bedroom')
    {
        return 'rgba(70, 132, 70, 0.6)';
    }
    else if (location === 'mechanical_room')
    {
        return 'rgba(41, 35, 232, 0.6)';
    }
    return 'rgba(50,50,50, 0.6)';
}

var getPointStyle = function(typeOfReading)
{
    if (typeOfReading === 'temperature')
    {
        return 'circle'
    }
    else if (typeOfReading === 'humidity')
    {
        return 'triangle';
    }
    else if (typeOfReading === 'pressure')
    {
        return 'rectRot';
    }
    return 'star';
}

var getLineWidth = function (location)
{
    if (location === 'office')
    {
        return 2;
    }
    else if (location === 'bedroom')
    {
        return 3;
    }
    return 6;
}