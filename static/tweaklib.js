var tweaklib = (function(){
	const STATUS_OK = 1;
	const STATUS_FAILURE = 2;
	const STATUS_CONNECTING = 3;

	var socket = null;
	var vars = {};
	var id_key = 1;

	function serialize(field){
		var $field = $(field);
		var datatype = $field.data('datatype');

		switch ( datatype ){
		case DATATYPE_INTEGER:
			return parseInt($field.find('input').val());

		case DATATYPE_FLOAT:
		case DATATYPE_DOUBLE:
			return parseFloat($field.find('input').val());

		case DATATYPE_TIME:
			return $field.data('time').serialize();

		default:
			return value;
		}
	}

	function apply_field_options(item, field){
		if ( !(field && item.options) ) return;

		switch ( item.datatype ){
		case DATATYPE_INTEGER:
		case DATATYPE_FLOAT:
		case DATATYPE_DOUBLE:
			if ( 'min' in item.options ){
				field.attr('min', item.options.min);
			}
			if ( 'max' in item.options ){
				field.attr('max', item.options.max);
			}
			if ( 'step' in item.options ){
				field.attr('step', item.options.step);
			}
			break;
		}
	}

	function template_filename(datatype){
		switch ( datatype ){
		default: return false;
		}
	}

	function render_template(datatype, data){
		var filename = template_filename(datatype);
		if ( !filename ){
			return $('<div class="form-group"><input type="number" class="form-control" /></div>');
		}
		return $(Handlebars.templates[filename](data));
	}

	function render_var(key){
		var item = vars[key];

		if ( item.elem ){
			/* only update value without recreating element */
			/* @todo handle change of datatype */
			item.field.val(item.value);
			return;
		}

		item.elem = $('<div></div>');
		item.elem.data('name', key);
		item.elem.attr('data-name', key); /* for easier debugger with inspector */
		item.elem.addClass('var-' + (id_key++));
		item.elem.append('<h3>' + item.name + '</h3>');
		if ( item.description ){
			item.elem.find('h3').append(' <small>' + item.description + '</small>');
		}

		var field = null;
		switch ( item.datatype ){
		case DATATYPE_INTEGER:
		case DATATYPE_FLOAT:
		case DATATYPE_DOUBLE:
			field = render_template(item.datatype);
			field.find('input').val(item.value);
			break;

		default:
			console.log('Unknown datatype', item.datatype);
		}

		if ( field ){
			field.data('datatype', item.datatype);
			apply_field_options(item, field);

			field.find('input').change(function(){
				send_update(item);
			});
			item.elem.append(field);
			item.field = field;
		}

		$('#vars').append(item.elem);
	}

	function send_update(item){
		socket.send(JSON.stringify({
			type: 'update',
			handle: item.handle,
			value: serialize(item.field),
		}));
	}

	function load_vars(data){
		for ( var key in data ){
			var elem = data[key];
			vars[elem.name] = $.extend({}, vars[elem.name], elem);
			render_var(elem.name);
		}
	}

	function set_status(msg, type){
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
				init();
			});
			status.append(button);
		}

	}

	function init(){
		set_status('Connecting', STATUS_CONNECTING);
		socket = new WebSocket("ws://localhost:8080/socket", "v1.tweaklib.sidvind.com");

		socket.onopen = function(event){
			set_status('Connected', STATUS_OK);
		};

		socket.onerror = function(event){
			console.log(event);
			set_status('Disconnected', STATUS_FAILURE);
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
