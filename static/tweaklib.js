var tweaklib = (function(){
	const STATUS_OK = 1;
	const STATUS_FAILURE = 2;
	const STATUS_CONNECTING = 3;

	var socket = null;
	var vars = {};
	var id_key = 1;

	Handlebars.registerHelper('field-attributes', function(context) {
		var options = context.data.root;
		var attr = [];

		for ( var key in options.attributes ){
			attr.push(key + '="' + Handlebars.Utils.escapeExpression(options.attributes[key]) + '"');
		}

		return new Handlebars.SafeString(attr.join(' '));
	});

	function Field(datatype, options){
		this.datatype = datatype;
		this.element = this.create(options);
		this.bind();
	};

	Field.prototype = {
		/**
		 * Get/Set the value of this field.
		 */
		value: function(value){
			var e = this.element.find('input');
			return e.val.apply(e, arguments);
		},

		/**
		 * Serialize the data so it can be sent over the socket to the application.
		 */
		serialize: function(){
			return value;
		},

		/**
		 * Take serialized data and update the field with the new data.
		 */
		unserialize: function(data){
			this.value(data);
		},

		/**
		 * Create DOM elements.
		 */
		create: function(options){
			var html = this.template(options);
			html.data('field', this);
			return html;
		},

		/**
		 * Return the template filename (in src/templates, remember to recompile after updating)
		 */
		template_filename: function(datatype){
			return 'default.html';
		},

		/**
		 * Load template and return DOM.
		 */
		template: function(options){
			var filename = this.template_filename(this.datatype);
			return $(Handlebars.templates[filename]({
				attributes: this.filter_attributes(options),
			}));
		},

		/**
		 * Returns a list of allowed options/attributes that the user can set on this field type.
		 */
		allowed_attributes: function(){
			return [];
		},

		/**
		 * Filter out the allowed set of attributes/options for this field type.
		 */
		filter_attributes: function(options){
			var tmp = {};
			var allowed = this.allowed_attributes();

			for ( var key in options ){
				if ( allowed.indexOf(key) >= 0 ){
					tmp[key] = options[key];
				}
			}

			return tmp;
		},

		/**
		 * Setup bindings.
		 */
		bind: function(){
			this.element.find('input').change(function(){
				send_update(item);
			});
		},
	};

	function NumericalField(datatype, options) {
		Field.call(this, datatype, options);
	}

	NumericalField.prototype = $.extend(Object.create(Field.prototype), {
		constructor: NumericalField,

		serialize: function(){
			switch ( this.datatype ){
			case DATATYPE_INTEGER:
				return parseInt(this.value());

			case DATATYPE_FLOAT:
			case DATATYPE_DOUBLE:
				return parseFloat(this.value());

			default:
				return value;
			}
		},

		allowed_attributes: function(){
			return ['min', 'max', 'step'];
		},
	});

	Field.factory = function(datatype, options){
		switch (datatype){
		case DATATYPE_INTEGER:
		case DATATYPE_FLOAT:
		case DATATYPE_DOUBLE:
			return new NumericalField(datatype, options);
		default:
			return new Field(datatype, options);
		}
	};

	function render_var(item){
		if ( item.elem ){
			/* only update value without recreating element */
			/* @todo handle change of datatype */
			item.field.unserialize(item.value);
			return;
		}

		item.elem = $('<div></div>');
		item.elem.data('handle', item.handle);
		item.elem.attr('data-handle', item.handle); /* for easier debugger with inspector */
		item.elem.addClass('var-' + (id_key++));
		item.elem.append('<h3>' + item.name + '</h3>');
		if ( item.description ){
			item.elem.find('h3').append(' <small>' + item.description + '</small>');
		}

		var field = Field.factory(item.datatype, item.options);
		field.unserialize(item.value);

		item.elem.append(field.element);
		item.field = field;

		$('#vars').append(item.elem);
	}

	function send_update(item){
		socket.send(JSON.stringify({
			type: 'update',
			handle: item.handle,
			value: item.field.serialize(),
		}));
	}

	function load_vars(data){
		for ( var key in data ){
			var elem = data[key];
			vars[elem.handle] = item = $.extend({}, vars[elem.handle], elem);
			render_var(item);
		}
	}

	function var_from_handle(handle){
		return vars[handle];
	}

	function refresh_vars(data){
		for ( var key in data ){
			var elem = data[key];
			var item = var_from_handle(elem.handle);
			item.field.unserialize(elem.value);
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

			if ( data.type === 'hello' ){
				load_vars(data.vars);
			} else if ( data.type == 'refresh' ){
				refresh_vars(data.vars);
			} else {
				console.log('message', data);
			}
		}
	}

	init();
})();
