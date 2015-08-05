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
