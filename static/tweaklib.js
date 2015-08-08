var tweaklib = (function(){
	const STATUS_OK = 1;
	const STATUS_FAILURE = 2;
	const STATUS_CONNECTING = 3;

	var socket = null;
	var vars = {};
	var id_key = 1;
	var factory = {};
	var files = ['/handlebars.runtime-v3.0.3.js', '/templates.js', '/constants.js', '/tweaklib/field.js', '/tweaklib/numerical.js', '/tweaklib/variable.js'];
	var tasks = []; /* use add_task() to push loading tasks */
	var handlers = {};

	function var_from_handle(handle){
		return vars[handle];
	}

	function create_vars(data){
		for ( var key in data ){
			var elem = data[key];
			vars[elem.handle] = new Variable($.extend({}, vars[elem.handle], elem));
		}
	}

	function update_vars(data){
		for ( var key in data ){
			var elem = data[key];
			var item = var_from_handle(elem.handle);
			item.unserialize(elem.value);
			item.render();
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
				connect();
			});
			status.append(button);
		}
	}

	function init_handlebars(dfn){
		Handlebars.registerHelper('field-attributes', function(context) {
			var options = context.data.root;
			var attr = [];

			for ( var key in options.attributes ){
				attr.push(key + '="' + Handlebars.Utils.escapeExpression(options.attributes[key]) + '"');
			}

			return new Handlebars.SafeString(attr.join(' '));
		});
		dfn.resolve();
	}

	handlers.hello = function(data){
		create_vars(data.vars);
		update_vars(data.vars);
	};

	handlers.refresh = function(data){
		update_vars(data.vars);
	};

	function connect(){
		var dfn = $.Deferred();

		set_status('Connecting', STATUS_CONNECTING);
		socket = new WebSocket("ws://localhost:8080/socket", "v1.tweaklib.sidvind.com");

		socket.onopen = function(event){
			set_status('Connected', STATUS_OK);
		};

		socket.onerror = function(event){
			console.log(event);
			set_status('Disconnected', STATUS_FAILURE);
			dfn.fail();
		}

		socket.onclose = function(event){
			set_status('Disconnected', STATUS_FAILURE);
		}

		socket.onmessage = function(event){
			var data = JSON.parse(event.data);

			if ( data.type in handlers ){
				handlers[data.type](data);
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
	connect.message = "Connecting to application";

	function update_progress(i, msg){
		var n = Math.ceil(i / (tasks.length-1) * 100);
		var p = n + '%';
		$('#loading .progress-bar').attr('aria-valuenow', n).css('width', p).html(p);
		$('#loading p').html(msg);
	}

	function add_task(func){
		if ( Array.isArray(func) ){
			for ( var i in func ){
				add_task(func[i]);
			}
			return;
		}

		var n = tasks.length;
		tasks.push(function(){
			update_progress(n, func.message);
			return func();
		});
	}

	function wrap_task(func, message, data){
		var wrapped = function(){
			var dfn = $.Deferred();
			setTimeout(function(){
				func(dfn, data);
			}, 0);
			return dfn.promise();
		};
		wrapped.message = message;
		return wrapped;
	}

	function load_script(filename){
		/* kind of similar to jQuery.getScript but inserts to head so
		 * filenames/line-numbers is preserved, making debugging easier. */
		/** @todo rumors says this wont work in IE */
		var dfn = $.Deferred();
		var script = document.createElement('script');
		document.head.appendChild(script);
		script.type = 'text/javascript';
		script.onload = function(){ dfn.resolve(); };
		script.onerror = function(){ dfn.fail(); };
		script.src = filename;
		return dfn.promise();
	}

	function init(){
		add_task($.map(files, function(filename){
			var func = function(){ return load_script(filename); };
			func.message = 'Loading ' + filename;
			return func;
		}));
		add_task(wrap_task(init_handlebars, 'Initializing handlebars library'));
		add_task(connect);

		var loader = $.Deferred();

		var dfn = tasks.reduce(function(prev,cur){
			return prev.then(cur);
		}, loader);

		dfn.then(function(){
			setTimeout(function(){
				$('#loading').remove();
				$('#vars').show();
			}, 500);
		});

		dfn.fail(function(){
			$('#loading p').addClass('alert alert-danger').prepend('Task failed: ');
		});

		loader.resolve();
	}

	return {
		init: init,
		factory: factory,

		send: function(data){
			socket.send(JSON.stringify(data));
		},

		register_field: function(datatype, callback){
			if ( !Array.isArray(datatype) ){
				datatype = [datatype];
			}

			for ( var key in datatype ){
				var dt = datatype[key];
				factory[datatype[key]] = function(options){
					return callback(dt, options);
				};
			}
		},
	};
})();

$(function(){
	tweaklib.init();
});
