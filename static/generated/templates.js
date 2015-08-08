(function() {
  var template = Handlebars.template, templates = Handlebars.templates = Handlebars.templates || {};
templates['default.html'] = template({"compiler":[6,">= 2.0.0-beta.1"],"main":function(depth0,helpers,partials,data) {
    var stack1, helper;

  return "<div class=\"form-group\">\n	<input type=\"number\" class=\"form-control\" "
    + ((stack1 = ((helper = (helper = helpers['field-attributes'] || (depth0 != null ? depth0['field-attributes'] : depth0)) != null ? helper : helpers.helperMissing),(typeof helper === "function" ? helper.call(depth0,{"name":"field-attributes","hash":{},"data":data}) : helper))) != null ? stack1 : "")
    + " />\n</div>\n";
},"useData":true});
templates['wrapper.html'] = template({"1":function(depth0,helpers,partials,data) {
    var helper;

  return "<small> "
    + this.escapeExpression(((helper = (helper = helpers.description || (depth0 != null ? depth0.description : depth0)) != null ? helper : helpers.helperMissing),(typeof helper === "function" ? helper.call(depth0,{"name":"description","hash":{},"data":data}) : helper)))
    + "</small>";
},"compiler":[6,">= 2.0.0-beta.1"],"main":function(depth0,helpers,partials,data) {
    var stack1, helper, alias1=helpers.helperMissing, alias2="function", alias3=this.escapeExpression;

  return "<div data-handle=\""
    + alias3(((helper = (helper = helpers.handle || (depth0 != null ? depth0.handle : depth0)) != null ? helper : alias1),(typeof helper === alias2 ? helper.call(depth0,{"name":"handle","hash":{},"data":data}) : helper)))
    + "\" id=\"var-"
    + alias3(((helper = (helper = helpers.handle || (depth0 != null ? depth0.handle : depth0)) != null ? helper : alias1),(typeof helper === alias2 ? helper.call(depth0,{"name":"handle","hash":{},"data":data}) : helper)))
    + "\" class=\"dt-"
    + ((stack1 = (helpers['datatype-name'] || (depth0 && depth0['datatype-name']) || alias1).call(depth0,(depth0 != null ? depth0.datatype : depth0),{"name":"datatype-name","hash":{},"data":data})) != null ? stack1 : "")
    + "\">\n	<h3>"
    + alias3(((helper = (helper = helpers.name || (depth0 != null ? depth0.name : depth0)) != null ? helper : alias1),(typeof helper === alias2 ? helper.call(depth0,{"name":"name","hash":{},"data":data}) : helper)))
    + ((stack1 = helpers['if'].call(depth0,(depth0 != null ? depth0.description : depth0),{"name":"if","hash":{},"fn":this.program(1, data, 0),"inverse":this.noop,"data":data})) != null ? stack1 : "")
    + "</h3>\n</div>\n";
},"useData":true});
})();
