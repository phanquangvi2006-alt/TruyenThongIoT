const coap = require('coap');

const server = coap.createServer();

server.on('request', (req, res) => {
    console.log('Received:', req.payload.toString());
    res.end('OK'); // Comment dòng này trong lần test đầu
});

server.listen(() => {
    console.log('CoAP server started');
});
