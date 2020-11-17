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