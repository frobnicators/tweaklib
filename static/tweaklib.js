var tweaklib = (function(){
	var socket = new WebSocket("ws://localhost:8080/socket", "tweaklib/1.0");
	socket.send('test');
})();
