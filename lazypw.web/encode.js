var _encode = Module.cwrap('encode', // name of C function
'string', // return type
['string']); // argument types


function encode(text) {
    if (text) return _encode(text);
    return "";
}