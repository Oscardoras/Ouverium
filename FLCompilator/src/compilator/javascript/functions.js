function Pointer(variable) {
    this.variable = variable;
}

//Assign function (:=)
function __a90__a93(parameters) {
    var variables = parameters[0];
    var objects = parameters[1];

    if (variables instanceof Pointer) {
        variables.variable = objects;
    } else if (variables instanceof Array) {
        for (var i = 0; i < variables.length; i++) {
            if (variables[i] instanceof Pointer) {
                variables[i].variable = objects[i];
            } else {
                __assign([variables[i], objects[1]]);
            }
        }
    }
}