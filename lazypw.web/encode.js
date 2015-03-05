var _encode = Module.cwrap('encode', // name of C function
'string', // return type
['string']); // argument types


function encode(text) {
    var ret = _encode(text);
    return ret;
}