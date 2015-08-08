(function(){
	'use strict';

	function NumericalField(datatype, options) {
		Field.call(this, datatype, options);
	}

	NumericalField.prototype = $.extend(Object.create(Field.prototype), {
		constructor: NumericalField,

		serialize: function(){
			switch ( this.datatype ){
			case constants.DATATYPE_INTEGER:
				return parseInt(this.value());

			case constants.DATATYPE_FLOAT:
			case constants.DATATYPE_DOUBLE:
				return parseFloat(this.value());

			default:
				return this.value();
			}
		},

		allowed_attributes: function(){
			return ['min', 'max', 'step'];
		},
	});

	tweaklib.register_field([constants.DATATYPE_INTEGER, constants.DATATYPE_FLOAT, constants.DATATYPE_DOUBLE], function(datatype, options){
		return new NumericalField(datatype, options);
	});
})();
