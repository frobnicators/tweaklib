(function() {
  var template = Handlebars.template, templates = Handlebars.templates = Handlebars.templates || {};
templates['default.html'] = template({"compiler":[6,">= 2.0.0-beta.1"],"main":function(depth0,helpers,partials,data) {
    var stack1, helper;

  return "<div class=\"form-group\">\n	<input type=\"number\" class=\"form-control\" "
    + ((stack1 = ((helper = (helper = helpers['field-attributes'] || (depth0 != null ? depth0['field-attributes'] : depth0)) != null ? helper : helpers.helperMissing),(typeof helper === "function" ? helper.call(depth0,{"name":"field-attributes","hash":{},"data":data}) : helper))) != null ? stack1 : "")
    + " />\n</div>\n";
},"useData":true});
})();
