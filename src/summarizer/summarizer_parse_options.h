/*******************************************************************\

Module: Command Line Parsing

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_SUMMARIZER_PARSE_OPTIONS_H
#define CPROVER_SUMMARIZER_PARSE_OPTIONS_H

#include <util/ui_message.h>
#include <util/parse_options.h>

#include <langapi/language_ui.h>

class goto_modelt;
class optionst;

#include "summary_checker.h"

#define SUMMARIZER_OPTIONS \
  "(summarize)" \
  "(xml-ui)(html-report):" \
  "(function):" \
  "D:I:" \
  "(depth):(context-bound):(unwind):" \
  "(bounds-check)(pointer-check)(div-by-zero-check)(memory-leak-check)" \
  "(signed-overflow-check)(unsigned-overflow-check)" \
  "(float-overflow-check)(nan-check)" \
  "(no-assertions)(no-assumptions)" \
  "(16)(32)(64)(LP64)(ILP64)(LLP64)(ILP32)(LP32)" \
  "(little-endian)(big-endian)" \
  "(error-label):(verbosity):(no-library)" \
  "(version)" \
  "(i386-linux)(i386-macos)(i386-win32)(win32)(winx64)(gcc)" \
  "(ppc-macos)(unsigned-char)" \
  "(string-abstraction)(no-arch)(arch):(floatbv)(fixedbv)" \
  "(round-to-nearest)(round-to-plus-inf)(round-to-minus-inf)(round-to-zero)" \
  "(inline)" \
  "(show-locs)(show-vcc)(show-properties)(show-trace)(show-fixed-points)" \
  "(show-goto-functions)(show-guards)(show-defs)(show-ssa)(show-assignments)" \
  "(show-value-sets)" \
  "(property):(all-properties)" \
  "(no-simplify)(no-fixed-point)" \
  "(storefront-alarms):" \
  "(horn-encoding):" \
  "(no-unwinding-assertions)(no-propagation)"
  // the last line is for CBMC-regression testing only

class summarizer_parse_optionst:
  public parse_options_baset,
  public language_uit
{
public:
  virtual int doit();
  virtual void help();

  summarizer_parse_optionst(int argc, const char **argv);
  summarizer_parse_optionst(
    int argc,
    const char **argv,
    const std::string &extra_options);

protected:
  void get_command_line_options(optionst &options);

  bool get_goto_program(
    const optionst &options,
    goto_modelt &goto_model);

  bool process_goto_program(
    const optionst &options,
    goto_modelt &goto_model);
    
  bool set_properties(goto_modelt &);

  void report_success();
  void report_failure();

  void report_properties(
    const goto_modelt &,
    const summary_checkert::property_mapt &);  

  void show_counterexample(
    const goto_modelt &,
    const class goto_tracet &);
            
  void eval_verbosity();
  
  void require_entry(const goto_modelt &);
};

#endif
