function Pointer(variable) {
    this.value = value;
}

//Assign function (:=)
function __a90__a93(parameters) {
    var variables = parameters[0];
    var objects = parameters[1];

    if (variables instanceof Pointer) {
        variables.value = objects;
    } else if (variables instanceof Array) {
        for (var i = 0; i < variables.length; i++) {
            if (variables[i] instanceof Pointer) {
                variables[i].value = objects[i];
            } else {
                __a90__a93([variables[i], objects[1]]);
            }
        }
    }
}