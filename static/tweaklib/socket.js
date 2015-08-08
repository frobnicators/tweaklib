var TweakSocket = (function(){
	const STATUS_OK = 1;
	const STATUS_FAILURE = 2;
	const STATUS_CONNECTING = 3;
	const PROTOCOL = 'v1.tweaklib.sidvind.com';

	function TweakSocket(handlers){
		this.handlers = handlers;
		this.socket = undefined;
	}

	TweakSocket.prototype.connect = function(){
		var dfn = $.Deferred();
		var self = this;

		this.set_status('Connecting', STATUS_CONNECTING);
		this.socket = new WebSocket(this.get_url(), PROTOCOL);

		this.socket.onopen = function(event){
			self.set_status('Connected', STATUS_OK);
		}

		this.socket.onerror = function(event){
			console.log(event);
			self.set_status('Disconnected', STATUS_FAILURE);
			dfn.fail();
		}

		this.socket.onclose = function(event){
			self.set_status('Disconnected', STATUS_FAILURE);
		}

		this.socket.onmessage = function(event){
			var data = JSON.parse(event.data);

			if ( data.type in self.handlers ){
				self.handlers[data.type](data);
			} else {
				console.log('no handler for ' + data.type + ', ignored');
			}

			/* hack for init message */
			if ( data.type === 'hello' ){
				dfn.resolve();
			}
		}

		return dfn.promise();
	}

	TweakSocket.prototype.send = function(data){
		this.socket.send(data);
	}

	TweakSocket.prototype.get_url = function(){
		return 'ws://' + window.location.host + '/socket';
	}

	TweakSocket.prototype.set_status = 	function set_status(msg, type){
		var status = $('#status');
		var var_container = $('#vars');
		status.html('<p>' + msg + '</p>');
		status.removeClass('status-ok status-failure');
		var_container.removeClass('failure');

		switch (type){
		case STATUS_OK:
			status.addClass('status-ok');
			break;
		case STATUS_FAILURE:
		case STATUS_CONNECTING:
			status.addClass('status-failure');
			var_container.addClass('failure');
			break;
		}

		if ( type == STATUS_FAILURE ){
			var button = $('<button class="btn btn-default">Reconnect</button>');
			button.click(function(){
				$(this).hide();
				set_status('Connecting', STATUS_CONNECTING);
				connect();
			});
			status.append(button);
		}
	};

	return TweakSocket;
}());
