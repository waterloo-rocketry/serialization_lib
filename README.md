# Waterloo Rocketry Serialization Library

There are several rocketry electrical systems that need to communicate
with each other over a UART connection (either a direct UART
connection, or a UART->radio transmitter->receiver->UART) in a ASCII
printable fashion. As such, we needed a library to encode arbitrary
structures into some (hopefully data efficient) ASCII formats.

Unfortunately, some of the systems that need to communicate are 8-bit
microcontrollers that don't support dynamic memory allocation or
recursion, and only work on the C90 standard. Most serialization
libraries we found didn't work within these restrictions, so we
decided to write our own. This is that.

## Serializing

You can serialize an arbitrary structure using the `wsdl_serialize`
function, like so:

```
struct {
    ....
} data;

// Buffer to hold serialized data
char ser[WSDL_SER_LEN(sizeof(data))];

// Serialize the contents of data into ser
size_t len = wsdl_serialize(&data, sizeof(data), ser, sizeof(ser));

// Transmit serialized version
radio_send(ser, len);
```

## Deserializing

A serialized string can be deserialized either all at once, or byte by
byte. To deserialize a full string all at once, do the following.

```
struct {
    ...
} data;

const char *ser = "...";

// Deserialize the contents of ser into data
wsdl_deserialize(&data, sizeof(data), ser, strlen(ser));
```

This works if the whole serialized string is known in
advance. However, when reading directly from a UART interface, it's
better to deal with the serialized string one byte at a time, so that
the microcontroller doesn't have to buffer all of the incoming bytes
before decoding them. This can be done like so:

```
struct {
    ...
} data;

// Context variable to remember how much we've deserialized so far
wsdl_ctx_t context;
wsdl_begin_deserialization(&context, &data, sizeof(data));

while(1) {
    if (radio_bytes_available()) {
        if (!wsdl_deserialize_byte(&context, radio_read_byte())) {
            // finished deserializing, `data` now holds deserialized input
            ...
        }
    }
    
    // Do other things while waiting for more bytes from the radio
    ...
```
