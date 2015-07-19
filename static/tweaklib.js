var tweaklib = (function(){
	var socket = new WebSocket("ws://localhost:8080/socket", "v1.tweaklib.sidvind.com");
	socket.onopen = function(event){
		console.log('opened', event);
		socket.send('test');
	};
	socket.onmessage = function(event){
		console.log('message', event.data);
	};
	console.log(socket);
})();
