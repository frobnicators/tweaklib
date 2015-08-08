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
		var self = this;
		this.element.find('input').change(function(){
			self.send_update();
		});
	},

	get_handle: function(){
		return this.item.handle;
	},

	send_update: function(){
		tweaklib.send({
			type: 'update',
			handle: this.get_handle(),
			value: this.serialize(),
		});
	},
};
