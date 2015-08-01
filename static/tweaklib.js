var tweaklib = (function(){
	const DATATYPE_INTEGER = 0;
	const STATUS_OK = 1;
	const STATUS_FAILURE = 2;

	var socket = null;
	var vars = {};
	var id_key = 1;

	function serialize(value, field){
		var datatype = $(field).data('datatype');

		switch ( datatype ){
		case DATATYPE_INTEGER:
			return parseInt(value);

		default:
			return value;
		}
	}

	function render_var(key){
		var item = vars[key];
		if ( item.elem ) return;

		item.elem = $('<div></div>');
		item.elem.data('name', key);
		item.elem.addClass('var-' + (id_key++));
		item.elem.append('<h3>' + item.name + '</h3>');
		if ( item.description ){
			item.elem.find('h3').append(' <small>' + item.description + '</small>');
		}

		var field = null;
		switch ( item.datatype ){
		case DATATYPE_INTEGER:
			field = $('<input type="number" class="form-control" />');
			field.val(item.value);
			break;

		default:
			console.log('Unknown datatype', item.datatype);
		}
		field.data('datatype', item.datatype);

		if ( field ){
			field.change(function(){
				var value = $(this).val();
				socket.send(JSON.stringify({
					type: 'update',
					handle: item.handle,
					value: serialize(value, this),
				}));
			});
			item.elem.append(field);
		}

		$('#vars').append(item.elem);
	}

	function load_vars(data){
		for ( var key in data ){
			var elem = data[key];
			vars[elem.name] = elem;
			render_var(elem.name);
		}
	}

	function set_status(msg, type){
		var status = $('#status');
		status.html(msg);
		status.removeClass('status-ok status-failure');

		switch (type){
		case STATUS_OK:
			status.addClass('status-ok'); break;
		case STATUS_FAILURE:
			status.addClass('status-failure'); break;
		}
	}

	function init(){
		socket = new WebSocket("ws://localhost:8080/socket", "v1.tweaklib.sidvind.com");

		socket.onopen = function(event){
			set_status('Connected', STATUS_OK);
		};

		socket.onerror = function(event){
			console.log(event);
		}

		socket.onclose = function(event){
			set_status('Disconnected', STATUS_FAILURE);
		}

		socket.onmessage = function(event){
			var data = JSON.parse(event.data);
			console.log('message', data);

			if ( data.type === 'hello' ){
				load_vars(data.vars);
			};
		}
	}

	init();
})();
