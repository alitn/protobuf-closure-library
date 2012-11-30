// Copyright (c) 2010-2011 SameGoal LLC.
// All Rights Reserved.
// Author: Andy Hochhaus <ahochhaus@samegoal.com>

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "code_generator.h"

#include <string>
#include <vector>
#include <iostream>  // NOLINT
#include <sstream>  // NOLINT

#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/stubs/common.h"

#include "int64_encoding.pb.h"
#include "javascript_package.pb.h"

namespace sg {
namespace protobuf {
namespace js {

CodeGenerator::CodeGenerator(const std::string &name)
    : name_(name) {}

CodeGenerator::~CodeGenerator() {}

std::string CodeGenerator::js_superclass_ = "goog.proto2.Message";
std::string CodeGenerator::js_model_superclass_ = "";
std::string CodeGenerator::js_collection_superclass_ = "";
bool CodeGenerator::advanced_ = false;

bool CodeGenerator::Generate(
    const google::protobuf::FileDescriptor *file,
    const std::string & parameter ,
    google::protobuf::compiler::OutputDirectory *output_directory,
    std::string *error) const {

  std::vector<std::pair<std::string, std::string> > options;
  google::protobuf::compiler::ParseGeneratorParameter(parameter, &options);
  
  for (unsigned int i = 0; i < options.size(); i++) {
    if (options[i].first == "js_superclass") {
      CodeGenerator::js_superclass_ = options[i].second;
    } else if (options[i].first == "js_model_superclass") {
      CodeGenerator::js_model_superclass_ = options[i].second;
    } else if (options[i].first == "js_collection_superclass") {
      CodeGenerator::js_collection_superclass_ = options[i].second;
    } else if (options[i].first == "advanced" && options[i].second == "true") {
      CodeGenerator::advanced_ = true;
    } else {
      *error = "Unknown generator option: " + options[i].first;
      return false;
    }
  }

  const std::string file_name = file->name();
  std::string output_file_name = file->name();
  std::size_t loc = output_file_name.rfind(".");
  output_file_name.erase(loc, output_file_name.length() - loc);
  output_file_name.append(".pb.js");

  google::protobuf::internal::scoped_ptr<
      google::protobuf::io::ZeroCopyOutputStream> output(
          output_directory->Open(output_file_name));
  google::protobuf::io::Printer printer(output.get(), '$');
  printer.Print(
      "// Generated by the protocol buffer compiler.  DO NOT EDIT!\n");
  printer.Print("// source: $file_name$\n", "file_name", file_name);
  printer.Print("\n");
  printer.Print(
      "/**\n"
      " * @fileoverview Generated Protocol Buffer code for file $file_name$.\n"
      " */\n", "file_name", file_name);
  printer.Print("\n");

  // goog.provide all messages and enums
  for (int i = 0; i < file->message_type_count(); ++i) {
    CodeGenerator::GenDescriptorGoogProvides(
        file->message_type(i),
        &printer);
  }
  for (int i = 0; i < file->enum_type_count(); ++i) {
    CodeGenerator::GenEnumDescriptorGoogProvides(
        file->enum_type(i),
        &printer);
  }

  printer.Print("\n");
  printer.Print("goog.require('$js_superclass$');\n",
                "js_superclass", CodeGenerator::js_superclass_);
  if (CodeGenerator::advanced_) {
    printer.Print("goog.require('$js_model_superclass$');\n",
                  "js_model_superclass", CodeGenerator::js_model_superclass_);
    printer.Print("goog.require('$js_collection_superclass$');\n",
                  "js_collection_superclass", CodeGenerator::js_collection_superclass_);
  }
  printer.Print("\n");
  for (int i = 0; i < file->dependency_count(); ++i) {
    for (int j = 0; j < file->dependency(i)->message_type_count(); j++) {
      printer.Print(
          "goog.require('$file$');\n",
          "file",
          JsFullName(file->dependency(i)->message_type(j)->file(),
                     file->dependency(i)->message_type(j)->full_name()));
    }
  }

  printer.Print("\n");

  // generate accessor functions
  for (int i = 0; i < file->message_type_count(); ++i) {
    CodeGenerator::GenDescriptor(
        file->message_type(i),
        &printer);
  }
  for (int i = 0; i < file->enum_type_count(); ++i) {
    CodeGenerator::GenEnumDescriptor(
        file->enum_type(i),
        &printer);
  }

  printer.Print("\n");

  // generate metadata
  for (int i = 0; i < file->message_type_count(); ++i) {
    CodeGenerator::GenDescriptorMetadata(
        file->message_type(i),
        &printer);
  }

  if (printer.failed()) {
    *error = "CodeGenerator detected write error.";
    return false;
  }

  return true;
}

std::string CodeGenerator::JsFullName(
    const google::protobuf::FileDescriptor *file,
    const std::string &full_name) {
  std::string new_name = full_name;
  const std::string package = file->package();

  std::string prefix = file->options().GetExtension(javascript_package);
  if (prefix.length() > 0) {
    if (package.length() > 0) {
      new_name.erase(0, package.length() + 1);
    }
    new_name = prefix + "." + new_name;
  }
  return new_name;
}

void CodeGenerator::GenDescriptorGoogProvides(
    const google::protobuf::Descriptor *message,
    google::protobuf::io::Printer *printer) {
  printer->Print("goog.provide('$name$');\n",
                 "name", JsFullName(message->file(),
                                    message->full_name()));

  // enums
  for (int i = 0; i < message->enum_type_count(); ++i) {
    CodeGenerator::GenEnumDescriptorGoogProvides(
        message->enum_type(i),
        printer);
  }

  // Recursively process nested messages
  for (int i = 0; i < message->nested_type_count(); ++i) {
    CodeGenerator::GenDescriptorGoogProvides(
        message->nested_type(i),
        printer);
  }
}

void CodeGenerator::GenEnumDescriptorGoogProvides(
    const google::protobuf::EnumDescriptor *enum_desc,
    google::protobuf::io::Printer *printer) {
  printer->Print("goog.provide('$name$');\n",
                 "name", JsFullName(enum_desc->file(),
                                    enum_desc->full_name()));
}

void CodeGenerator::GenDescriptor(
    const google::protobuf::Descriptor *message,
    google::protobuf::io::Printer *printer) {
  if (CodeGenerator::advanced_) {
    // model
    CodeGenerator::GenModel(message, printer);
  }

  printer->Print("\n"
                 "/**\n"
                 " * Message$for_model$ $name$.\n"
                 " * @constructor\n"
                 " * @extends {$js_superclass$}\n"
                 " */\n",
                 "name", message->name(),
                 "for_model", CodeGenerator::advanced_ ? " for model" : "",
                 "js_superclass", CodeGenerator::js_superclass_);
  printer->Print("$name$$message$ = function() {\n",
                 "name", JsFullName(message->file(),
                                    message->full_name()),
                 "message", CodeGenerator::advanced_ ? ".Message" : "");
  printer->Indent();
  printer->Print("$js_superclass$.call(this);\n",
                 "js_superclass", CodeGenerator::js_superclass_);
  printer->Outdent();
  printer->Print("};\n"
                 "goog.inherits($name$$message$, $js_superclass$);\n"
                 "\n"
                 "\n",
                 "name", JsFullName(message->file(),
                                    message->full_name()),
                 "message", CodeGenerator::advanced_ ? ".Message" : "",
                  "js_superclass", CodeGenerator::js_superclass_);

  printer->Print(
      "/**\n"
      " * Overrides {@link $js_superclass$#clone} to specify its exact "
      "return type.\n"
      " * @return {!$name$} The cloned message.\n"
      " * @override\n"
      " */\n"
      "$name$$message$.prototype.clone;\n",
      "js_superclass", CodeGenerator::js_superclass_,
      "name", JsFullName(message->file(),
                         message->full_name()),
      "message", CodeGenerator::advanced_ ? ".Message" : "");

  // fields
  for (int i = 0; i < message->field_count(); ++i) {
    CodeGenerator::GenFieldDescriptor(
        message->field(i),
        printer);
  }

  // enums
  for (int i = 0; i < message->enum_type_count(); ++i) {
    CodeGenerator::GenEnumDescriptor(
        message->enum_type(i),
        printer);
  }

  // nested messages (recursively process)
  for (int i = 0; i < message->nested_type_count(); ++i) {
    CodeGenerator::GenDescriptor(
        message->nested_type(i),
        printer);
    printer->Print("\n"
                   "\n");
  }

  if (CodeGenerator::advanced_) {
    // collection
    CodeGenerator::GenCollection(message, printer);
  }
}

void CodeGenerator::GenFieldDescriptor(
    const google::protobuf::FieldDescriptor *field,
    google::protobuf::io::Printer *printer) {
  printer->Print("\n");
  std::string type;
  bool type_is_primitive = false;
  bool as_number = field->options().GetExtension(jstype);
  if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) {
    type = "boolean";
    type_is_primitive = true;
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES ||
      field->type() == google::protobuf::FieldDescriptor::TYPE_STRING ||
      ((field->type() == google::protobuf::FieldDescriptor::TYPE_INT64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_SINT64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_FIXED64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_SFIXED64) &&
       !as_number)) {
    type = "string";
    type_is_primitive = true;
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_GROUP ||
      field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
    type = JsFullName(field->message_type()->file(),
                      field->message_type()->full_name());
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
    type = JsFullName(field->enum_type()->file(),
                      field->enum_type()->full_name());
    type_is_primitive = true;
  } else {
    type = "number";
    type_is_primitive = true;
  }

  std::ostringstream number;
  number << field->number();

  std::string upper_name = field->camelcase_name();
  if (upper_name[0] >= 97 && upper_name[0] <= 122) {
    upper_name[0] -= 32;
  }

  // get
  if (field->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
    printer->Print(
        "\n"
        "/**\n"
        " * Gets the value of the $name$ field at the index given.\n"
        " * @param {number} index The index to lookup.\n",
        "name", field->name());
    printer->Print(
        " * @return {$opt$$type$} The value.\n"
        " */\n",
        "opt", type_is_primitive ? "?" : "",
        "type", type);
    printer->Print(
        "$prefix$.prototype.get$field$ = function(index) {\n",
        "prefix", JsFullName(field->containing_type()->file(),
                             field->containing_type()->full_name()),
        "field", upper_name);
    printer->Indent();
    printer->Print(
        "return /** @type {$opt$$type$} */ ",
        "opt", type_is_primitive ? "?" : "",
        "type", type);
    printer->Print(
        "(this.get$$Value($number$, index));\n",
        "number", number.str());
    printer->Outdent();
    printer->Print(
        "};\n"
        "\n");

    printer->Print(
        "\n"
        "/**\n"
        " * Gets the value of the $name$ field at the index given or the "
            "default value if not set.\n"
        " * @param {number} index The index to lookup.\n",
        "name", field->name());
    printer->Print(
        " * @return {$opt$$type$} The value.\n"
        " */\n",
        "opt", type_is_primitive ? "" : "!",
        "type", type);
    printer->Print(
        "$prefix$.prototype.get$field$OrDefault = function(index) {\n",
        "prefix", JsFullName(field->containing_type()->file(),
                             field->containing_type()->full_name()),
        "field", upper_name);
    printer->Indent();
    printer->Print(
        "return /** @type {$opt$$type$} */ (",
        "opt", type_is_primitive ? "" : "!",
        "type", type);
    printer->Print(
        "this.get$$ValueOrDefault($number$, "
            "index));\n",
        "number", number.str());
    printer->Outdent();
    printer->Print(
        "};\n"
        "\n");
  } else {
    printer->Print(
        "\n"
        "/**\n"
        " * Gets the value of the $name$ field.\n",
        "name", field->name());
    printer->Print(
        " * @return {$opt$$type$} The value.\n"
        " */\n",
        "opt", type_is_primitive ? "?" : "",
        "type", type);
    printer->Print(
        "$prefix$.prototype.get$field$ = function() {\n",
        "prefix", JsFullName(field->containing_type()->file(),
                             field->containing_type()->full_name()),
        "field", upper_name);
    printer->Indent();
    printer->Print(
        "return /** @type {$opt$$type$} */ ",
        "opt", type_is_primitive ? "?" : "",
        "type", type);
    printer->Print(
        "(this.get$$Value($number$));\n",
        "number", number.str());
    printer->Outdent();
    printer->Print(
        "};\n"
        "\n");

    printer->Print(
        "\n"
        "/**\n"
        " * Gets the value of the $name$ field or the default value if not "
        "set.\n",
        "name", field->name());
    printer->Print(
        " * @return {$opt$$type$} The value.\n"
        " */\n",
        "opt", type_is_primitive ? "" : "!",
        "type", type);
    printer->Print(
        "$prefix$.prototype.get$field$OrDefault = function() {\n",
        "prefix", JsFullName(field->containing_type()->file(),
                             field->containing_type()->full_name()),
        "field", upper_name);
    printer->Indent();
    printer->Print(
        "return /** @type {$opt$$type$} */ (",
        "opt", type_is_primitive ? "" : "!",
        "type", type);
    printer->Print(
        "this.get$$ValueOrDefault($number$));\n",
        "number", number.str());
    printer->Outdent();
    printer->Print(
        "};\n"
        "\n");
  }

  // set
  if (field->label() != google::protobuf::FieldDescriptor::LABEL_REPEATED) {
    printer->Print("\n"
                   "/**\n"
                   " * Sets the value of the $name$ field.\n",
                   "name", field->name());
    printer->Print(" * @param {$opt$$type$} value The value.\n"
                   " * @param {Object=} opt_options Options.\n"
                   " */\n",
                   "opt", type_is_primitive ? "" : "!",
                   "type", type);
    printer->Print("$prefix$.prototype.set$field$ = function(value, opt_options) {\n",
                   "prefix", JsFullName(field->containing_type()->file(),
                                        field->containing_type()->full_name()),
                   "field", upper_name);
    printer->Indent();
    printer->Print(
        "this.set$$Value($number$, value, opt_options);\n",
        "number", number.str());
    printer->Outdent();
    printer->Print("};\n"
                   "\n");
  }

  // add, Array
  if (field->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
    printer->Print("\n"
                   "/**\n"
                   " * Adds a value to the $name$ field.\n",
                   "name", field->name());
    printer->Print(" * @param {$opt$$type$} value The value to add.\n"
                   " */\n",
                   "opt", type_is_primitive ? "" : "!",
                   "type", type);
    printer->Print("$prefix$.prototype.add$field$ = function(value) {\n",
                   "prefix", JsFullName(field->containing_type()->file(),
                                        field->containing_type()->full_name()),
                   "field", upper_name);
    printer->Indent();
    printer->Print(
        "this.add$$Value($number$, value);\n",
        "number", number.str());
    printer->Outdent();
    printer->Print("};\n"
                   "\n");

    printer->Print("\n"
                   "/**\n"
                   " * Returns the array of values in the $name$ field.\n",
                   "name", field->name());
    printer->Print(
        " * @return {!Array.<$opt$$type$>} The values in the field.\n"
        " */\n",
        "opt", type_is_primitive ? "" : "!",
        "type", type);
    printer->Print("$prefix$.prototype.$field$Array = function() {\n",
                   "prefix", JsFullName(field->containing_type()->file(),
                                        field->containing_type()->full_name()),
                   "field", field->camelcase_name());
    printer->Indent();
    printer->Print(
        "return /** @type {!Array.<$opt$$type$>} */ (",
        "type", type,
        "opt", type_is_primitive ? "" : "!");
    printer->Print(
        "this.array$$Values($number$));"
        "\n",
        "number", number.str());
    printer->Outdent();
    printer->Print("};\n"
                   "\n");
  }

  // has
  printer->Print("\n"
                 "/**\n"
                 " * @return {boolean} Whether the $name$ field has a value.\n"
                 " */\n",
                 "name", field->name());
  printer->Print("$prefix$.prototype.has$field$ = function() {\n",
                 "prefix", JsFullName(field->containing_type()->file(),
                                      field->containing_type()->full_name()),
                 "field", upper_name);
  printer->Indent();
  printer->Print("return this.has$$Value($number$);\n",
                 "number", number.str());
  printer->Outdent();
  printer->Print("};\n"
                 "\n");

  // count
  printer->Print(
      "\n"
      "/**\n"
      " * @return {number} The number of values in the $name$ field.\n"
      " */\n",
      "name", field->name());
  printer->Print("$prefix$.prototype.$field$Count = function() {\n",
                 "prefix", JsFullName(field->containing_type()->file(),
                                      field->containing_type()->full_name()),
                 "field", field->camelcase_name());
  printer->Indent();
  printer->Print("return this.count$$Values($number$);\n",
                 "number", number.str());
  printer->Outdent();
  printer->Print("};\n"
                 "\n");

  // clear
  printer->Print("\n"
                 "/**\n"
                 " * Clears the values in the $name$ field.\n"
                 " */\n",
                 "name", field->name());
  printer->Print("$prefix$.prototype.clear$field$ = function() {\n",
                 "prefix", JsFullName(field->containing_type()->file(),
                                      field->containing_type()->full_name()),
                 "field", upper_name);
  printer->Indent();
  printer->Print("this.clear$$Field($number$);\n",
                 "number", number.str());
  printer->Outdent();
  printer->Print("};\n");

  if (CodeGenerator::advanced_) {
    // update events
    printer->Print("\n"
                   "/**\n"
                   " * Listens to update event on $name$ field.\n"
                   " * @param {Function} callback The callback to invoke.\n"
                   " * @param {Object} context The 'this' sent to callback.\n"
                   " * @return {number} Listerner key.\n"
                   " */\n",
                   "name", field->name());
    printer->Print("$prefix$.prototype.onUpdate$field$ = function(callback, context) {\n",
                   "prefix", JsFullName(field->containing_type()->file(),
                                        field->containing_type()->full_name()),
                   "field", upper_name);
    printer->Indent();
    printer->Print("return this.listen(this.eventNameUpdate($number$), callback, context);\n",
                   "number", number.str());
    printer->Outdent();
    printer->Print("};\n");

    // collections
    if (field->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED &&
        field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
      printer->Print("\n"
                     "/**\n"
                     " * Returns the collection in the $name$ field.\n",
                     "name", field->name());
      printer->Print(
          " * @return {Object} The values in the field.\n"
          " */\n");
      printer->Print("$prefix$.prototype.$field$Collection = function() {\n",
                     "prefix", JsFullName(field->containing_type()->file(),
                                          field->containing_type()->full_name()),
                     "field", field->camelcase_name());
      printer->Indent();
      printer->Print(
          "return (this.message_.collection$$Values($number$));"
          "\n",
          "number", number.str());
      printer->Outdent();
      printer->Print("};\n"
                     "\n");
    }
  } // if (CodeGenerator::advanced_)
}

void CodeGenerator::GenEnumDescriptor(
    const google::protobuf::EnumDescriptor *enum_desc,
    google::protobuf::io::Printer *printer) {
  printer->Print("\n"
                 "\n"
                 "/**\n"
                 " * Enumeration $name$.\n"
                 " * @enum {number}\n"
                 " */\n",
                 "name", enum_desc->name());
  printer->Print("$name$ = {\n",
                 "name", JsFullName(enum_desc->file(),
                                    enum_desc->full_name()));
  printer->Indent();
  for (int i = 0; i < enum_desc->value_count(); ++i) {
    std::string format = "$key$: $value$,\n";
    if (i == enum_desc->value_count() - 1) {
      format = "$key$: $value$\n";
    }
    std::ostringstream number;
    number << enum_desc->value(i)->number();
    printer->Print(format.c_str(),
                   "key", enum_desc->value(i)->name(),
                   "value", number.str());
  }
  printer->Outdent();
  printer->Print("};\n"
                 "\n"
                 "\n");
}

void CodeGenerator::GenDescriptorMetadata(
      const google::protobuf::Descriptor *message,
      google::protobuf::io::Printer *printer) {
  printer->Print("\n"
                 "\n"
                 "$js_superclass$.set$$Metadata($name$$message$, {\n",
                 "js_superclass", CodeGenerator::js_superclass_,
                 "name", JsFullName(message->file(),
                                    message->full_name()),
                 "message", CodeGenerator::advanced_ ? ".Message" : "");
  printer->Indent();
  printer->Print("0: {\n");
  printer->Indent();
  printer->Print("name: '$name$',\n",
                 "name", message->name());
  if (message->containing_type() != NULL) {
    printer->Print("containingType: $type$$message$,\n",
                   "type",
                   JsFullName(message->containing_type()->file(),
                              message->containing_type()->full_name()),
                   "message", CodeGenerator::advanced_ ? ".Message" : "");
  }
  printer->Print("fullName: '$fullname$'\n",
                 "fullname", message->full_name());
  printer->Outdent();
  if (message->field_count() > 0) {
    printer->Print("},\n");
  } else {
    printer->Print("}\n");
  }
  // fields
  for (int i = 0; i < message->field_count(); ++i) {
    CodeGenerator::GenFieldDescriptorMetadata(
        message->field(i),
        printer);
    if (i != message->field_count() - 1) {
      printer->Print(",\n");
    } else {
      printer->Print("");
    }
  }
  printer->Print("}\n");
  if (CodeGenerator::advanced_) {
    // model type
    printer->Print(", $full_name$\n", "full_name", message->full_name());
    // collection type
    printer->Print(", $full_name$Collection\n", "full_name", message->full_name());
  }
  printer->Outdent();
  printer->Print(");\n");

  // nested messages (recursively process)
  for (int i = 0; i < message->nested_type_count(); ++i) {
    CodeGenerator::GenDescriptorMetadata(
        message->nested_type(i),
        printer);
  }
}

void CodeGenerator::GenFieldDescriptorMetadata(
      const google::protobuf::FieldDescriptor *field,
      google::protobuf::io::Printer *printer) {
  std::ostringstream number;
  number << field->number();

  std::ostringstream default_value;
  std::string js_type;
  bool as_number = field->options().GetExtension(jstype);
  if (field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE) {
    js_type = "DOUBLE";
    default_value << field->default_value_double();
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT) {
    js_type = "FLOAT";
    default_value << field->default_value_float();
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT64) {
    js_type = "INT64";
    if (as_number) {
      default_value << field->default_value_int64();
    } else {
      default_value << "'" << field->default_value_int64() << "'";
    }
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64) {
    js_type = "UINT64";
    if (as_number) {
      default_value << field->default_value_uint64();
    } else {
      default_value << "'" << field->default_value_uint64() << "'";
    }
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT32) {
    js_type = "INT32";
    default_value << field->default_value_uint32();
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_FIXED64) {
    js_type = "FIXED64";
    if (as_number) {
      default_value << field->default_value_uint64();
    } else {
      default_value << "'" << field->default_value_uint64() << "'";
    }
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_FIXED32) {
    js_type = "FIXED32";
    default_value << field->default_value_uint32();
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) {
    js_type = "BOOL";
    default_value << field->default_value_bool();
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) {
    js_type = "STRING";
    default_value << "'" << field->default_value_string() << "'";
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_GROUP) {
    js_type = "GROUP";
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
    js_type = "MESSAGE";
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES) {
    js_type = "BYTES";
    default_value << "'" << field->default_value_string() << "'";
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32) {
    js_type = "UINT32";
    default_value << field->default_value_uint32();
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
    js_type = "ENUM";

    default_value << JsFullName(field->enum_type()->file(),
                                field->enum_type()->full_name() + "." +
                                field->default_value_enum()->name());
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_SFIXED32) {
    js_type = "SFIXED32";
    default_value << field->default_value_int32();
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_SFIXED64) {
    js_type = "SFIXED64";
    if (as_number) {
      default_value << field->default_value_int64();
    } else {
      default_value << "'" << field->default_value_int64() << "'";
    }
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_SINT32) {
    js_type = "SINT32";
    default_value << field->default_value_int32();
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_SINT64) {
    js_type = "SINT64";
    if (as_number) {
      default_value << field->default_value_int64();
    } else {
      default_value << "'" << field->default_value_int64() << "'";
    }
  }

  std::string js_object;
  if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) {
    js_object = "Boolean";
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES ||
      field->type() == google::protobuf::FieldDescriptor::TYPE_STRING ||
      ((field->type() == google::protobuf::FieldDescriptor::TYPE_INT64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_SINT64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_FIXED64 ||
        field->type() == google::protobuf::FieldDescriptor::TYPE_SFIXED64) &&
       !as_number)) {
    js_object = "String";
  } else if (
      field->type() == google::protobuf::FieldDescriptor::TYPE_GROUP ||
      field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
    js_object = JsFullName(field->message_type()->file(),
                           field->message_type()->full_name()) +
                (CodeGenerator::advanced_ ? ".Message" : "");
  } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
    js_object = JsFullName(field->enum_type()->file(),
                           field->enum_type()->full_name());
  } else {
    js_object = "Number";
  }

  printer->Print("$number$: {\n",
                 "number", number.str());
  printer->Indent();
  printer->Print("name: '$name$',\n",
                 "name", field->name());
  if (field->is_repeated()) {
    printer->Print("repeated: true,\n");
  }
  printer->Print("fieldType: $js_superclass$.FieldType.$js_type$,\n",
                 "js_superclass", CodeGenerator::js_superclass_,
                 "js_type", js_type);
  if (field->has_default_value() ||
      field->type() == google::protobuf::FieldDescriptor::TYPE_ENUM) {
    printer->Print("defaultValue: $default$,\n",
                   "default", default_value.str());
  }
  printer->Print("type: $type$\n",
                 "type", js_object);
  printer->Outdent();
  printer->Print("}");
}

void CodeGenerator::GenModel(
      const google::protobuf::Descriptor *message,
      google::protobuf::io::Printer *printer) {
  printer->Print("\n"
                 "/**\n"
                 " * Model $name$.\n"
                 " * @param {Object=} opt_attribs Attributes.\n"
                 " * @constructor\n"
                 " * @extends {$js_model_superclass$}\n"
                 " */\n",
                 "name", message->name(),
                 "js_model_superclass", CodeGenerator::js_model_superclass_);
  printer->Print("$name$ = function(opt_attribs) {\n",
                 "name", JsFullName(message->file(),
                                  message->full_name()));
  printer->Indent();
  printer->Print("$js_model_superclass$.call(this, opt_attribs);\n",
                 "js_model_superclass", CodeGenerator::js_model_superclass_);
  printer->Outdent();
  printer->Print("};\n"
                 "goog.inherits($name$, $js_model_superclass$);\n"
                 "\n"
                 "\n",
                 "name", JsFullName(message->file(),
                                    message->full_name()),
                  "js_model_superclass", CodeGenerator::js_model_superclass_);
}

void CodeGenerator::GenCollection(
      const google::protobuf::Descriptor *message,
      google::protobuf::io::Printer *printer) {
  printer->Print("\n"
                 "/**\n"
                 " * Collection for $name$.\n"
                 " * @constructor\n"
                 " * @extends {$js_collection_superclass$}\n"
                 " */\n",
                 "name", message->name(),
                 "js_collection_superclass", CodeGenerator::js_collection_superclass_);
  printer->Print("$name$Collection = function() {\n",
                 "name", JsFullName(message->file(),
                                  message->full_name()));
  printer->Indent();
  printer->Print("$js_collection_superclass$.apply(this);\n",
                 "js_collection_superclass", CodeGenerator::js_collection_superclass_);
  printer->Outdent();
  printer->Print("};\n"
                 "goog.inherits($name$Collection, $js_collection_superclass$);\n"
                 "\n"
                 "\n",
                 "name", JsFullName(message->file(),
                                    message->full_name()),
                  "js_collection_superclass", CodeGenerator::js_collection_superclass_);
}

}  // namespace js
}  // namespace protobuf
}  // namespace sg
