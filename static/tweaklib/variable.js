function Variable(item){
	this.datatype = item.datatype;
	this.name = item.name;
	this.handle = item.handle;
	this.description = item.description;
	this.options = item.options;
	this.render();
};

Variable.prototype.render = function(value){
	if ( this.field ){
		/* @todo handle change of datatype */
		return;
	}

	this.field = tweaklib.factory[this.datatype](this.options);
	this.field.item = this;
	this.element = this.template();
	this.element.append(this.field.element);
	$('#vars').append(this.element);
}

Variable.prototype.template = function(){
	return $(Handlebars.templates['wrapper.html'](this));
}

Variable.prototype.unserialize = function(data){
	this.field.unserialize(data);
}

Variable.prototype.send_update = function(){
	this.field.send_update();
}
