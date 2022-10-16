function Reference(value) {
    this.value = value;
}

//Assign function (:=)
var __a58__a61 = new Reference(function(parameters) {
    var variables = parameters[0];
    var objects = parameters[1];

    if (variables instanceof Pointer) {
        variables.value = objects.value;
    } else if (variables instanceof Array) {
        for (var i = 0; i < variables.length; i++) {
            if (variables[i] instanceof Pointer) {
                variables[i].value = objects[i].value;
            } else {
                __a90__a93([variables[i], objects[1]]);
            }
        }
    }

    return parameters[0];
});

//Def function (:)
var __a58 = new Pointer(function(parameters) {
    parameters[0].value = parameters[1].value;
    return parameters[0];

    /*
    var previous = parameters[0].value;

    parameters[0] = function(param) {
        if ((parameters[1].value.domain)(param)) (parameters[1].value)(param);
        else previous(param);
    }
    */
});

//Equality function (=)
var __a61 = new Pointer(function(parameters) {
    return new Pointer(parameters[0].value == parameters[1].value);
});

//Inequality function (!=)
var __a33__a61 = new Pointer(function(parameters) {
    return new Pointer(parameters[0].value != parameters[1].value);
});

//Modulo function (%)
var __a37 = new Pointer(function(parameters) {
    return new Pointer(parameters[0].value % parameters[1].value);
});

//Print function (print)
var print = new Pointer(function(parameters) {
    console.log(parameters.value);
});