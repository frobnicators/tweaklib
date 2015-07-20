var tweaklib = (function(){
	const DATATYPE_INTEGER = 0;

	var socket = null;
	var vars = {};
	var id_key = 1;

	function render_var(key){
		var item = vars[key];
		if ( item.elem ) return;

		item.elem = $('<div></div>');
		item.elem.data('name', key);
		item.elem.addClass('var-' + (id_key++));
		item.elem.append('<h3>' + item.name + '</h3>');
		if ( item.description ){
			item.elem.find('h3').append('<small>' + item.description + '</small>');
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

		if ( field ){
			field.change(function(){
				var value = $(this).val();
				socket.send(JSON.stringify({
					type: 'update',
					name: key,
					value: value,
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

	function init(){
		socket = new WebSocket("ws://localhost:8080/socket", "v1.tweaklib.sidvind.com");

		socket.onopen = function(event){
			console.log('opened', event);
			socket.send('test');
		};

		socket.onmessage = function(event){
			var data = JSON.parse(event.data);
			console.log('message', data);
			console.log(data.type);

			if ( data.type === 'hello' ){
				load_vars(data.vars);
			};
		}
	}

	init();
})();
