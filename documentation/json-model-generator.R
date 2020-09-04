
flatten <- function (...) unlist(list(...));
strcat <- function (...) paste0(flatten(...), collapse = '');
strreplace <- function (x, a, b) paste0(strsplit(x, a)[[1]], collapse = b);
catline <- function (...) cat(flatten(...), '\n', sep = '');

setClass("include", slots = list(header = "character", comments = "character"));
setClass("typedef", slots = list(alias = "character", original = "character"));
setClass("field", slots = list(type = "character", name = "character", is.required = "logical",
  default.value = "character", comments = "character",
  do.pass.by.value = "logical", do.json = "logical", is.range = "logical",
  is.typedef = "logical"));

make.include <- function(header, comments = "") new("include", header = header, comments = comments);
make.typedef <- function(alias, original) new("typedef", alias = alias, original = original);
make.field <- function(type, name, is.required, default.value = "{}", comments = "", do.pass.by.value = FALSE, do.json = TRUE)
  new("field",
    type = type,
    name = name,
    is.required = is.required,
    default.value = default.value,
    comments = comments,
    do.pass.by.value = do.pass.by.value,
    do.json = do.json,
    is.range = FALSE,
    is.typedef = FALSE);

format.includes <- function(includes) {
  n <- length(includes);
  headers <- rep("", times = n);
  has.comments <- rep(FALSE, times = n);
  padded.width <- 0;
  
  for (i in 1 : n) {
    headers[i] <- includes[[i]]@header;
    has.comments[i] <- (nchar(includes[[i]]@comments) > 0);
    if (has.comments[i]) padded.width <- max(padded.width, nchar(headers[i]));
  } # for (...)
  
  # ~~ Remove duplicates ~~
  mask <- !duplicated(headers);
  includes <- includes[mask];
  n <- length(includes);
  headers <- headers[mask];
  has.comments <- has.comments[mask];
  
  # ~~ Pad and join headers & comments ~~
  for (i in 1 : n) if (!has.comments[i]) headers[i] <- strcat("#include ", headers[i]);
  for (i in 1 : n) if (has.comments[i]) headers[i] <- strcat(
    "#include ",
    headers[i],
    rep(' ', times = padded.width - nchar(headers[i])),
    " // ",
    includes[[i]]@comments);
  
  # ~~ Sort and group the headers ~~
  permutation <- sort(headers, index.return = TRUE)$ix;
  nocomment.headers <- NULL;
  commented.headers <- NULL;
  for (i in 1 : n) {
    j <- permutation[i];
    if (has.comments[j]) commented.headers <- append(commented.headers, headers[j])
    else nocomment.headers <- append(nocomment.headers, headers[j]);
  } # if (...)
  return(c(nocomment.headers, "", commented.headers));
}; # function (...)

json.model <- function (struct.name,
    namespace, template.declaration, template.names, options = list()) {
  
  tab.size <- 4;
  one.tab <- strcat(rep(' ', times = tab.size));
  two.tabs <- strcat(rep(one.tab, 2));
  three.tabs <- strcat(rep(one.tab, 3));
  four.tabs <- strcat(rep(one.tab, 4));
  
  has.template <- (template.declaration != "");
  templated.name <- struct.name;
  if (has.template) templated.name <- strcat(struct.name, "<", template.names, ">");
  qualified.name <- strcat(namespace, "::", templated.name);
  
  include.guard <- toupper(strcat(
    strreplace(namespace, "::", "_"),
    "_", struct.name, "_HPP_INCLUDED"));
  
  if (is.null(options$includes)) options$includes <- list();
  if (is.null(options$typedefs)) options$typedefs <- list();
  if (is.null(options$fields)) options$fields <- list();
  if (is.null(options$trivial.layout)) options$trivial.layout <- FALSE;
  if (is.null(options$setters)) options$setters <- TRUE;
  if (is.null(options$equatable)) options$equatable <- TRUE;
  if (is.null(options$validation.placeholder)) options$validation.placeholder <- FALSE;
  if (is.null(options$hash.placeholder)) options$hash.placeholder <- FALSE;
  
  if (options$trivial.layout) {
    if (options$validation.placeholder) {
      warning("Validation not supported when using trivial layout.");
      options$validation.placeholder <- FALSE;
    } # if (...)
    if (options$setters) {
      warning("Setters not supported when using trivial layout.");
      options$setters <- FALSE;
    } # if (...)
  } # if (...)
  
  # ~~ Add includes ~~
  options$includes <- append(options$includes, list(
    make.include("<nlohmann/json.hpp>"),
    make.include("<ropufu/noexcept_json.hpp>"),
    make.include("<stdexcept>", comments = "std::runtime_error"),
    make.include("<string_view>", comments = "std::string_view")));
  if (options$validation.placeholder)
    options$includes <- append(options$includes, list(
      make.include("<optional>", comments = "std::optional, std::nullopt"),
      make.include("<string>", comments = "std::string")));
  if (options$hash.placeholder)
    options$includes <- append(options$includes, list(
      make.include("<cstddef>", comments = "std::size_t"),
      make.include("<functional>", comments = "std::hash")));
  # ~~ Sort and format includes ~~
  includes <- format.includes(options$includes);
  
  # ~~ Mark field types as typedefs ~~
  for (typedef in options$typedefs) {
    for (i in 1 : length(options$fields)) {
      x <- options$fields[[i]];
      if (x@type == typedef@alias) {
        x@is.typedef <- TRUE;
        options$fields[[i]] <- x;
      } # if (...)
    } # for (...)
  } # for (...)
  
  # ~~ Preamble ~~
  catline();
  catline("#ifndef ", include.guard);
  catline("#define ", include.guard);
  catline();
  
  # ~~ Includes ~~
  for (x in includes) catline(x);
  catline();
  
  # ~~ Namespace ~~
  catline("namespace ", namespace);
  catline("{");
  
  # ~~ Forward declarations ~~
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "struct ", struct.name, ";");
  catline(one.tab);
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "void to_json(nlohmann::json& j, const ", templated.name, "& x) noexcept;");
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "void from_json(const nlohmann::json& j, ", templated.name, "& x);");
  catline(one.tab);
  
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "struct ", struct.name);
  catline(one.tab, "{");
  
  # ~~ Inside struct ~~
  catline(two.tabs, "using type = ", templated.name, ";");
  for (typedef in options$typedefs) catline(two.tabs, "using ", typedef@alias, " = ", typedef@original, ";");
  catline(two.tabs);
  
  # ~~ Json names ~~
  catline(two.tabs, "// ~~ Json names ~~");
  for (field in options$fields) {
    if (!field@do.json) next;
    pretty.name <- strreplace(field@name, "_", " ");
    catline(two.tabs, "static constexpr std::string_view jstr_", field@name, " = \"", pretty.name, "\";");
  }; # for (...)
  catline(two.tabs);
  catline(two.tabs, "friend ropufu::noexcept_json_serializer<type>;");
  catline(two.tabs);
  
  
  # ~~ Fields ~~
  field.prefix <- "";
  getter.suffix <- "";
  if (!options$trivial.layout){
    catline(one.tab, "private:");
    field.prefix <- "m_";
    getter.suffix <- "()";
  } # if (...)
  for (field in options$fields) {
    field.comments <- "";
    if (nchar(field@comments) > 0) field.comments <- strcat(" // ", field@comments);
    catline(two.tabs, field@type, " ", field.prefix, field@name, " = ", field@default.value, ";", field.comments);
  }; # for (...)
  
  # ~~ Validation block ~~
  if (options$validation.placeholder) {
    catline(two.tabs);
    catline(two.tabs, "/** @brief Validates the structure and returns an error message, if any. */");
    catline(two.tabs, "std::optional<std::string> error_message() const noexcept");
    catline(two.tabs, "{");
    catline(three.tabs, "/** @todo Add your validation messages here. */");
    catline(three.tabs, "return std::nullopt;");
    catline(two.tabs, "} // error_message(...)");
    catline(two.tabs);
    catline(two.tabs, "/** @exception std::logic_error Validation failed. */");
    catline(two.tabs, "void validate() const");
    catline(two.tabs, "{");
    catline(three.tabs, "std::optional<std::string> message = this->error_message();");
    catline(three.tabs, "if (message.has_value()) throw std::logic_error(message.value());");
    catline(two.tabs, "} // validate(...)");
  } # if (...)
    
  # ~~ Getters and maybe setters ~~
  if (!options$trivial.layout) {
    catline(one.tab);
    catline(one.tab, "public:");
    is.first <- TRUE;
    for (field in options$fields) {
      if (!is.first) catline(two.tabs);

      if (field@do.pass.by.value) filed.type <- field@type
      else filed.type <- strcat("const ", field@type, "&");
      
      catline(two.tabs, filed.type, " ", field@name, "() const noexcept");
      catline(two.tabs, "{");
      catline(three.tabs, "return this->", field.prefix, field@name, ";");
      catline(two.tabs, "} // ", field@name, "(...)");
      is.first <- FALSE;
      
      if (options$setters) {
        catline(two.tabs);
        if (options$validation.placeholder) catline(two.tabs, "/** @todo Add validation if necessary. */");
        catline(two.tabs, "void set_", field@name, "(", filed.type, " value) noexcept");
        catline(two.tabs, "{");
        catline(three.tabs, "this->", field.prefix, field@name, " = value;");
        if (options$validation.placeholder) catline(three.tabs, "// this->validate();");
        catline(two.tabs, "} // set_", field@name, "(...)");
      } # if (...)
    }; # for (...)
  } # if (...)
  
  # ~~ Equivalence ~~
  if (options$equatable) {
    catline(one.tab);
    catline(two.tabs, "/** Checks if this object is equivalent to \\param other. */");
    catline(two.tabs, "bool operator ==(const type& other) const noexcept");
    catline(two.tabs, "{");
    
    if (is.null(options$fields)) catline(three.tabs, "return true;")
    else catline(three.tabs, "return");
    
    for.index <- 0;
    for (field in options$fields) {
      for.index <- for.index + 1;
      
      eq.suffix <- " &&";
      if (for.index == length(options$fields)) eq.suffix = ";";

      catline(four.tabs, "this->", field.prefix, field@name, " == other.", field.prefix, field@name, eq.suffix);
    }; # for (...)
    catline(two.tabs, "} // operator ==(...)");
    catline(two.tabs);
    catline(two.tabs, "/** Checks if this object is not equivalent to \\param other. */");
    catline(two.tabs, "bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }");
  } # if (...)
  
  # ~~ Close struct ~~
  catline(one.tab, "}; // struct ", struct.name);
  catline(one.tab);

  
  # ~~ To JSON ~~
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "void to_json(nlohmann::json& j, const ", templated.name, "& x) noexcept");
  catline(one.tab, "{");
  catline(two.tabs, "using type = ", templated.name, ";");
  catline(two.tabs, "j = nlohmann::json{");
  json.pairs <- NULL;
  for (field in options$fields) {
    if (!field@do.json) next;
    json.pairs <- append(json.pairs,
      strcat(three.tabs, "{type::jstr_", field@name, ", x.", field@name, getter.suffix, "}"));
  } # for (...)
  catline(paste(json.pairs, sep = '', collapse = ",\n"));
  catline(two.tabs, "};");
  catline(one.tab, "} // to_json(...)");
  catline(one.tab);
  
  # ~~ From JSON ~~
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "void from_json(const nlohmann::json& j, ", templated.name, "& x)");
  catline(one.tab, "{");
  catline(two.tabs, "if (!noexcept_json::try_get(j, x))");
  catline(three.tabs, "throw std::runtime_error(\"Parsing <", struct.name, "> failed: \" + j.dump());");
  catline(one.tab, "} // from_json(...)");

  catline("} // namespace ", namespace);
  catline();

  
  # ~~ Serializer specialization ~~
  catline("namespace ropufu");
  catline("{");
  
  if (has.template) catline(one.tab, "template <", template.declaration, ">");
  catline(one.tab, "struct noexcept_json_serializer<", qualified.name, ">");
  catline(one.tab, "{");
  catline(two.tabs, "using result_type = ", qualified.name, ";");
  catline(two.tabs);
  catline(two.tabs, "static bool try_get(const nlohmann::json& j, result_type& x) noexcept");
  catline(two.tabs, "{");
  
  for (field in options$fields) {
    if (!field@do.json) next;
    json.method <- "optional";
    if (field@is.required) json.method <- "required";
    catline(three.tabs, "if (!noexcept_json::", json.method, "(j, result_type::jstr_", field@name, ", x.", field.prefix, field@name, ")) return false;");
  }; # for (...)
  catline(three.tabs, "return true;");
  
  catline(two.tabs, "} // try_get(...)");
  catline(one.tab, "}; // struct noexcept_json_serializer<...>");
  
  catline("} // namespace ropufu");
  catline();
  
  # ~~ Hash specialization placeholder ~~
  if (options$hash.placeholder) {
    catline("namespace std");
    catline("{");
    
    catline(one.tab, "/** @todo Customize your hash computations. */");
    if (has.template) catline(one.tab, "template <", template.declaration, ">");
    catline(one.tab, "struct hash<", qualified.name, ">");
    catline(one.tab, "{");
    catline(two.tabs, "using argument_type = ", qualified.name, ";");
    catline(two.tabs, "using result_type = std::size_t;");
    catline(two.tabs);
    catline(two.tabs, "result_type operator ()(const argument_type& x) const noexcept");
    catline(two.tabs, "{");
    catline(three.tabs, "result_type result = 0;");
    catline(three.tabs);
    
    for (field in options$fields) {
      hash.name <- strcat(field@name, "_hash");
      field.type <- field@type;
      if (field@is.typedef) field.type <- strcat("typename argument_type::", field.type);
      catline(three.tabs, "std::hash<", field.type,"> ", hash.name, " = {};");
    }; # for (...)
    catline(three.tabs);
    offset.increment <- max(1, floor(32 / length(options$fields)));
    offset <- 0;
    for (field in options$fields) {
      hash.name <- strcat(field@name, "_hash");
      offset.action <- strcat(" << ", offset);
      if (offset == 0) offset.action <- "";
      catline(three.tabs, "result ^= (", hash.name, "(x.", field@name, getter.suffix, ")", offset.action, ");");
      offset <- offset + offset.increment;
    }; # for (...)
    catline(three.tabs);
    catline(three.tabs, "return result;");
    
    catline(two.tabs, "} // operator ()(...)");
    catline(one.tab, "}; // struct hash<...>");
    
    catline("} // namespace std");
    catline();
    
  } # if (...)
  
  
  catline("#endif // ", include.guard);
  catline();
};

#============================================================
# This file was generated in part by json-model-generator.R
#============================================================
json.options <- list();
json.options$includes <- list(
  make.include("<concepts>", comments = "std::floating_point"),
  make.include("<cstddef>", comments = "std::size_t"),
  make.include("<set>", comments = "std::set"),
  make.include("<string>", comments = "std::string"),
  make.include("<vector>", comments = "std::vector"),
  make.include("<ropufu/concepts.hpp>"),
  make.include("<ropufu/enum_array.hpp>"),
  make.include("\"modifier.hpp\""),
  make.include("\"modifier_target.hpp\""),
  make.include("\"unit.hpp\"")
);
json.options$typedefs <- list(
  make.typedef("integer_type", original = "t_integer_type"),
  make.typedef("probability_type", original = "t_probability_type"),
  make.typedef("unit_type", original = "combat::unit<integer_type, probability_type>"),
  make.typedef("modifier_type", original = "combat::modifier<integer_type, probability_type>"),
  make.typedef("filter_type", original = "aftermath::enum_array<modifier_target, std::set<std::string>>"),
  make.typedef("modifiers_by_level_type", original = "std::vector<aftermath::enum_array<modifier_target, modifier_type>>")
);
json.options$fields <- list(
  make.field("std::string", "category", is.required = TRUE, default.value = "\"\""),
  make.field("std::string", "description", is.required = FALSE, default.value = "\"\""),
  make.field("std::set<std::string>", "names", is.required = TRUE),
  make.field("bool", "every_round", is.required = FALSE, do.pass.by.value = TRUE),
  make.field("filter_type", "filter_unit_categories", is.required = FALSE),
  make.field("filter_type", "filter_unit_names", is.required = FALSE),
  make.field("modifiers_by_level_type", "modifiers", is.required = TRUE),
  make.field("std::size_t", "level", is.required = FALSE, default.value = "0", do.pass.by.value = TRUE,
    do.json = FALSE,
    comment = "One-based modifier index. Zero means modifier will not be applied.")
);
json.options$trivial.layout <- FALSE;
json.options$equatable <- TRUE;
json.options$setters <- TRUE;
json.options$validation.placeholder <- TRUE;
json.options$hash.placeholder <- FALSE;

json.model("filtered_modifier",
  namespace = "ropufu::settlers_offline::combat",
  template.declaration = "ropufu::integer t_integer_type, std::floating_point t_probability_type",
  template.names = "t_integer_type, t_probability_type", options = json.options);
#============================================================

cat # ~~m=^.^=m
