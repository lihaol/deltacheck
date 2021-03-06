/*******************************************************************\

Module: Summarizer Command Line Options Processing

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <util/string2int.h>
#include <util/config.h>
#include <util/language.h>
#include <util/options.h>
#include <util/memory_info.h>

#include <ansi-c/ansi_c_language.h>
#include <cpp/cpp_language.h>

#include <goto-programs/goto_convert_functions.h>
#include <goto-programs/show_properties.h>
#include <goto-programs/set_properties.h>
#include <goto-programs/remove_function_pointers.h>
#include <goto-programs/read_goto_binary.h>
#include <goto-programs/loop_ids.h>
#include <goto-programs/link_to_library.h>
#include <goto-programs/goto_inline.h>
#include <goto-programs/xml_goto_trace.h>
#include <goto-programs/remove_returns.h>

#include <analyses/goto_check.h>

#include <langapi/mode.h>

#include "../deltacheck/version.h"

#include "summarizer_parse_options.h"
#include "summary_checker.h"
#include "summarizer.h"
#include "show.h"
#include "horn_encoding.h"

/*******************************************************************\

Function: summarizer_parse_optionst::summarizer_parse_optionst

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

summarizer_parse_optionst::summarizer_parse_optionst(int argc, const char **argv):
  parse_options_baset(SUMMARIZER_OPTIONS, argc, argv),
  language_uit("Summarizer " DELTACHECK_VERSION, cmdline)
{
}
  
/*******************************************************************\

Function: summarizer_parse_optionst::eval_verbosity

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizer_parse_optionst::eval_verbosity()
{
  // this is our default verbosity
  int v=messaget::M_STATISTICS;
  
  if(cmdline.isset("verbosity"))
  {
    v=unsafe_string2int(cmdline.get_value("verbosity"));
    if(v<0)
      v=0;
    else if(v>10)
      v=10;
  }
  
  ui_message_handler.set_verbosity(v);
}

/*******************************************************************\

Function: summarizer_parse_optionst::get_command_line_options

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizer_parse_optionst::get_command_line_options(optionst &options)
{
  if(config.set(cmdline))
  {
    usage_error();
    exit(1);
  }

  if(cmdline.isset("debug-level"))
    options.set_option("debug-level", cmdline.get_value("debug-level"));

  if(cmdline.isset("unwindset"))
    options.set_option("unwindset", cmdline.get_value("unwindset"));

  // check array bounds
  if(cmdline.isset("bounds-check"))
    options.set_option("bounds-check", true);
  else
    options.set_option("bounds-check", false);

  // check division by zero
  if(cmdline.isset("div-by-zero-check"))
    options.set_option("div-by-zero-check", true);
  else
    options.set_option("div-by-zero-check", false);

  // check overflow/underflow
  if(cmdline.isset("signed-overflow-check"))
    options.set_option("signed-overflow-check", true);
  else
    options.set_option("signed-overflow-check", false);

  // check overflow/underflow
  if(cmdline.isset("unsigned-overflow-check"))
    options.set_option("unsigned-overflow-check", true);
  else
    options.set_option("unsigned-overflow-check", false);

  // check overflow on floats
  if(cmdline.isset("float-overflow-check"))
    options.set_option("float-overflow-check", true);
  else
    options.set_option("float-overflow-check", false);

  // check for NaN (not a number)
  if(cmdline.isset("nan-check"))
    options.set_option("nan-check", true);
  else
    options.set_option("nan-check", false);

  // check pointers
  if(cmdline.isset("pointer-check"))
    options.set_option("pointer-check", true);
  else
    options.set_option("pointer-check", false);

  // check for memory leaks
  if(cmdline.isset("memory-leak-check"))
    options.set_option("memory-leak-check", true);
  else
    options.set_option("memory-leak-check", false);

  // check assertions
  if(cmdline.isset("no-assertions"))
    options.set_option("assertions", false);
  else
    options.set_option("assertions", true);

  // use assumptions
  if(cmdline.isset("no-assumptions"))
    options.set_option("assumptions", false);
  else
    options.set_option("assumptions", true);

  // magic error label
  if(cmdline.isset("error-label"))
    options.set_option("error-label", cmdline.get_value("error-label"));
}

/*******************************************************************\

Function: summarizer_parse_optionst::doit

  Inputs:

 Outputs:

 Purpose: invoke main modules

\*******************************************************************/

int summarizer_parse_optionst::doit()
{
  if(cmdline.isset("version"))
  {
    std::cout << DELTACHECK_VERSION << std::endl;
    return 0;
  }

  //
  // command line options
  //

  optionst options;
  get_command_line_options(options);

  eval_verbosity();
  
  //
  // Print a banner
  //
  status() << "SUMMARIZER version " << DELTACHECK_VERSION << eom;

  register_language(new_ansi_c_language);
  register_language(new_cpp_language);

  goto_modelt goto_model;

  if(get_goto_program(options, goto_model))
    return 6;
    
  try
  {
    // options for various debug outputs
      
    if(cmdline.isset("show-ssa"))
    {
      bool simplify=!cmdline.isset("no-simplify");
      irep_idt function=cmdline.get_value("function");
      show_ssa(goto_model, function, simplify, std::cout, ui_message_handler);
      return 7;
    }

    if(cmdline.isset("show-fixed-points"))
    {
      bool simplify=!cmdline.isset("no-simplify");
      irep_idt function=cmdline.get_value("function");
      show_fixed_points(goto_model, function, simplify, std::cout, ui_message_handler);
      return 7;
    }

    if(cmdline.isset("show-defs"))
    {
      irep_idt function=cmdline.get_value("function");
      show_defs(goto_model, function, std::cout, ui_message_handler);
      return 7;
    }

    if(cmdline.isset("show-assignments"))
    {
      irep_idt function=cmdline.get_value("function");
      show_assignments(goto_model, function, std::cout, ui_message_handler);
      return 7;
    }

    if(cmdline.isset("show-guards"))
    {
      irep_idt function=cmdline.get_value("function");
      show_guards(goto_model, function, std::cout, ui_message_handler);
      return 7;
    }
    
    if(cmdline.isset("show-value-sets"))
    {
      irep_idt function=cmdline.get_value("function");
      show_value_sets(goto_model, function, std::cout, ui_message_handler);
      return 7;
    }
  
    if(cmdline.isset("horn-encoding"))
    {
      status() << "Horn-clause encoding" << eom;
      namespacet ns(symbol_table);
      
      std::string out_file=cmdline.get_value("horn-encoding");
      
      if(out_file=="-")
      {
        horn_encoding(goto_model, std::cout);
      }
      else
      {
        #ifdef _MSC_VER
        std::ofstream out(widen(out_file).c_str());
        #else
        std::ofstream out(out_file.c_str());
        #endif
        
        if(!out)
        {
          error() << "Failed to open output file "
                  << out_file << eom;
          return 1;
        }
        
        horn_encoding(goto_model, out);
      }
        
      return 7;
    }
    
    if(cmdline.isset("summarize"))
    {
      summarizert summarizer;
      
      summarizer.set_message_handler(get_message_handler());
      summarizer.simplify=!cmdline.isset("no-simplify");
      summarizer.fixed_point=!cmdline.isset("no-fixed-point");

      // do actual summarization
      if(cmdline.isset("function"))
        summarizer(goto_model, cmdline.get_value("function"));
      else
        summarizer(goto_model);
        
      return 0;
    }
    else
    {
      // otherwise we check properties
      summary_checkert summary_checker;
      
      summary_checker.set_message_handler(get_message_handler());
      summary_checker.simplify=!cmdline.isset("no-simplify");
      summary_checker.fixed_point=!cmdline.isset("no-fixed-point");
      
      if(cmdline.isset("function"))
        summary_checker.function_to_check=cmdline.get_value("function");

      if(cmdline.isset("show-vcc"))
      {
        std::cout << "VERIFICATION CONDITIONS:\n\n";
        summary_checker.show_vcc=true;
        summary_checker(goto_model);
        return 0;
      }
      
      // do actual analysis
      switch(summary_checker(goto_model))
      {
      case property_checkert::PASS:
        report_properties(goto_model, summary_checker.property_map);
        report_success();
        return 0;
      
      case property_checkert::FAIL:
        report_properties(goto_model, summary_checker.property_map);
        report_failure();
        return 10;
      
      default:
        return 8;
      }
    }
  }
  
  catch(const std::string error_msg)
  {
    error() << error_msg << messaget::eom;
    return 8;
  }

  catch(const char *error_msg)
  {
    error() << error_msg << messaget::eom;
    return 8;
  }

  #if 0                                         
  // let's log some more statistics
  debug() << "Memory consumption:" << messaget::endl;
  memory_info(debug());
  debug() << eom;
  #endif
}

/*******************************************************************\

Function: summarizer_parse_optionst::set_properties

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool summarizer_parse_optionst::set_properties(goto_modelt &goto_model)
{
  try
  {
    if(cmdline.isset("property"))
      ::set_properties(goto_model, cmdline.get_values("property"));
  }

  catch(const char *e)
  {
    error() << e << eom;
    return true;
  }

  catch(const std::string e)
  {
    error() << e << eom;
    return true;
  }
  
  catch(int)
  {
    return true;
  }
  
  return false;
}

/*******************************************************************\

Function: summarizer_parse_optionst::require_entry

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
  
void summarizer_parse_optionst::require_entry(
  const goto_modelt &goto_model)
{
  irep_idt entry_point=goto_model.goto_functions.entry_point();
      
  if(goto_model.symbol_table.symbols.find(entry_point)==symbol_table.symbols.end())
    throw "The program has no entry point; please complete linking";
}

/*******************************************************************\

Function: summarizer_parse_optionst::get_goto_program

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
  
bool summarizer_parse_optionst::get_goto_program(
  const optionst &options,
  goto_modelt &goto_model)
{
  if(cmdline.args.size()==0)
  {
    error() << "Please provide a program to verify" << eom;
    return true;
  }

  try
  {
    if(cmdline.args.size()==1 &&
       is_goto_binary(cmdline.args[0]))
    {
      status() << "Reading GOTO program from file" << eom;

      if(read_goto_binary(cmdline.args[0],
           goto_model, get_message_handler()))
        return true;
        
      config.set_from_symbol_table(goto_model.symbol_table);

      if(cmdline.isset("show-symbol-table"))
      {
        show_symbol_table();
        return true;
      }
    }
    else if(cmdline.isset("show-parse-tree"))
    {
      if(cmdline.args.size()!=1)
      {
        error() << "Please give one source file only" << eom;
        return true;
      }
      
      std::string filename=cmdline.args[0];
      
      #ifdef _MSC_VER
      std::ifstream infile(widen(filename).c_str());
      #else
      std::ifstream infile(filename.c_str());
      #endif
                
      if(!infile)
      {
        error() << "failed to open input file `" << filename << "'" << eom;
        return true;
      }
                              
      languaget *language=get_language_from_filename(filename);
                                                
      if(language==NULL)
      {
        error() << "failed to figure out type of file `" <<  filename << "'" << eom;
        return true;
      }
      
      language->set_message_handler(get_message_handler());
                                                                
      status("Parsing", filename);
  
      if(language->parse(infile, filename))
      {
        error() << "PARSING ERROR" << eom;
        return true;
      }
      
      language->show_parse(std::cout);
      return true;
    }
    else
    {
      // override --function
      config.main="";
    
      if(parse()) return true;
      if(typecheck()) return true;
      if(final()) return true;

      // we no longer need any parse trees or language files
      clear_parse();

      if(cmdline.isset("show-symbol-table"))
      {
        show_symbol_table();
        return true;
      }

      #if 0
      irep_idt entry_point=goto_model.goto_functions.entry_point();
      
      if(symbol_table.symbols.find(entry_point)==symbol_table.symbols.end())
      {
        error() << "No entry point; please provide a main function" << eom;
        return true;
      }
      #endif
      
      status() << "Generating GOTO Program" << eom;

      goto_convert(symbol_table, goto_model, ui_message_handler);
    }

    // finally add the library
    status() << "Adding CPROVER library" << eom;
    link_to_library(goto_model, ui_message_handler);

    if(process_goto_program(options, goto_model))
      return true;
  }

  catch(const char *e)
  {
    error() << e << eom;
    return true;
  }

  catch(const std::string e)
  {
    error() << e << eom;
    return true;
  }
  
  catch(int)
  {
    return true;
  }
  
  catch(std::bad_alloc)
  {
    error() << "Out of memory" << eom;
    return true;
  }
  
  return false;
}

/*******************************************************************\

Function: summarizer_parse_optionst::process_goto_program

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
  
bool summarizer_parse_optionst::process_goto_program(
  const optionst &options,
  goto_modelt &goto_model)
{
  try
  {
    // do partial inlining
    status() << "Partial Inlining" << eom;
    goto_partial_inline(goto_model, ui_message_handler);
    
    if(!cmdline.isset("summarize"))
    {
      // add generic checks
      status() << "Generic Property Instrumentation" << eom;
      goto_check(options, goto_model);
    }
    
    // recalculate numbers, etc.
    goto_model.goto_functions.update();

    // add loop ids
    goto_model.goto_functions.compute_loop_numbers();
    
    // if we aim to cover, replace
    // all assertions by false to prevent simplification
    
    if(cmdline.isset("cover-assertions"))
      make_assertions_false(goto_model);

    // show it?
    if(cmdline.isset("show-loops"))
    {
      show_loop_ids(get_ui(), goto_model);
      return true;
    }

    status() << "Function Pointer Removal" << eom;
    remove_function_pointers(
      goto_model, cmdline.isset("pointer-check"));

    // now do full inlining, if requested

    if(cmdline.isset("inline"))
    {
      status() << "Performing full inlining" << eom;
      goto_inline(goto_model, ui_message_handler);
    }
    else
    {
      // we don't use returns
      remove_returns(goto_model);
      goto_model.goto_functions.update();
    }

    label_properties(goto_model);

    if(cmdline.isset("show-properties"))
    {
      show_properties(goto_model, get_ui());
      return true;
    }

    if(set_properties(goto_model))
      return true;

    // show it?
    if(cmdline.isset("show-goto-functions"))
    {
      const namespacet ns(goto_model.symbol_table);
      goto_model.goto_functions.output(ns, std::cout);
      return true;
    }
  }

  catch(const char *e)
  {
    error() << e << eom;
    return true;
  }

  catch(const std::string e)
  {
    error() << e << eom;
    return true;
  }
  
  catch(int)
  {
    return true;
  }
  
  catch(std::bad_alloc)
  {
    error() << "Out of memory" << eom;
    return true;
  }
  
  return false;
}

/*******************************************************************\

Function: summarizer_parse_optionst::report_properties

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizer_parse_optionst::report_properties(
  const goto_modelt &goto_model,
  const property_checkert::property_mapt &property_map)
{
  for(property_checkert::property_mapt::const_iterator
      it=property_map.begin();
      it!=property_map.end();
      it++)
  {
    if(get_ui()==ui_message_handlert::XML_UI)
    {
      xmlt xml_result("result");
      xml_result.set_attribute("property", id2string(it->first));
      xml_result.set_attribute("status", property_checkert::as_string(it->second.result));
      std::cout << xml_result << "\n";
    }
    else
    {
      status() << "[" << it->first << "] "
               << it->second.location->source_location.get_comment()
               << ": "
               << property_checkert::as_string(it->second.result)
               << eom;
    }

    if(cmdline.isset("show-trace") &&
       it->second.result==property_checkert::FAIL)
      show_counterexample(goto_model, it->second.error_trace);
  }

  if(!cmdline.isset("property"))
  {
    status() << eom;

    unsigned failed=0;

    for(property_checkert::property_mapt::const_iterator
        it=property_map.begin();
        it!=property_map.end();
        it++)
      if(it->second.result==property_checkert::FAIL)
        failed++;
    
    status() << "** " << failed
             << " of " << property_map.size() << " failed"
             << eom;  
  }
  
  if(cmdline.isset("storefront-alarms"))
  {
    std::ofstream out(cmdline.get_value("storefront-alarms").c_str());
    if(!out)
    {
      error() << "failed to write to file "
              << cmdline.get_value("storefront-alarms") << eom;
    }
    else
    {
      status() << "writing results into "
               << cmdline.get_value("storefront-alarms") << eom;
    
      out << "<data>\n\n";

      for(property_checkert::property_mapt::const_iterator
          it=property_map.begin();
          it!=property_map.end();
          it++)
      {
        if(it->second.result!=property_checkert::FAIL) continue;

        out << "<property>\n";

        out << "  <id>";
        xmlt::escape(id2string(it->first), out);
        out << "</id>\n";
        
        const source_locationt &l=it->second.location->source_location;

        out << "  <message>";
        xmlt::escape(id2string(l.get_comment()), out);
        out << "</message>\n";

        out << "  <category>";
        xmlt::escape(id2string(l.get_property_class()), out);
        out << "</category>\n";

        out << "  <file>";
        xmlt::escape(id2string(l.get_file()), out);
        out << "</file>\n";

        out << "  <line>";
        xmlt::escape(id2string(l.get_line()), out);
        out << "</line>\n";
        
        out << "</property>\n\n";
      }
      
      out << "</data>\n";
    }
  }
}

/*******************************************************************\

Function: summarizer_parse_optionst::report_success

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizer_parse_optionst::report_success()
{
  result() << "VERIFICATION SUCCESSFUL" << eom;

  switch(get_ui())
  {
  case ui_message_handlert::PLAIN:
    break;
    
  case ui_message_handlert::XML_UI:
    {
      xmlt xml("cprover-status");
      xml.data="SUCCESS";
      std::cout << xml;
      std::cout << std::endl;
    }
    break;
    
  default:
    assert(false);
  }
}

/*******************************************************************\

Function: summarizer_parse_optionst::show_counterexample

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizer_parse_optionst::show_counterexample(
  const goto_modelt &goto_model,
  const goto_tracet &error_trace)
{
  const namespacet ns(goto_model.symbol_table);

  switch(get_ui())
  {
  case ui_message_handlert::PLAIN:
    std::cout << std::endl << "Counterexample:" << std::endl;
    show_goto_trace(std::cout, ns, error_trace);
    break;
  
  case ui_message_handlert::XML_UI:
    {
      xmlt xml;
      convert(ns, error_trace, xml);
      std::cout << xml << std::endl;
    }
    break;
  
  default:
    assert(false);
  }
}

/*******************************************************************\

Function: summarizer_parse_optionst::report_failure

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizer_parse_optionst::report_failure()
{
  result() << "VERIFICATION FAILED" << eom;

  switch(get_ui())
  {
  case ui_message_handlert::PLAIN:
    break;
    
  case ui_message_handlert::XML_UI:
    {
      xmlt xml("cprover-status");
      xml.data="FAILURE";
      std::cout << xml;
      std::cout << std::endl;
    }
    break;
    
  default:
    assert(false);
  }
}

/*******************************************************************\

Function: summarizer_parse_optionst::help

  Inputs:

 Outputs:

 Purpose: display command line help

\*******************************************************************/

void summarizer_parse_optionst::help()
{
  std::cout <<
    "\n"
    "* *  Summarizer " DELTACHECK_VERSION " - Copyright (C) 2014-2015 ";
    
  std::cout << "(" << (sizeof(void *)*8) << "-bit version)";
    
  std::cout << "   * *\n";
    
  std::cout <<
    "* *                    Daniel Kroening                      * *\n"
    "* *                 University of Oxford                    * *\n"
    "* *                 kroening@kroening.com                   * *\n"
    "\n"
    "Usage:                       Purpose:\n"
    "\n"
    " summarizer [-?] [-h] [--help] show help\n"
    " summarizer file.c ...        source file names\n"
    " summarizer file.o ...        object file names\n"
    "\n"
    "Summarizer has two modes of operation:\n"
    " summarizer --summarize       compute summaries\n"
    " summarizer --check           check properties\n"
    "\n"
    "Frontend options:\n"
    " -I path                      set include path (C/C++)\n"
    " -D macro                     define preprocessor macro (C/C++)\n"
    " --preprocess                 stop after preprocessing\n"
    " --16, --32, --64             set width of int\n"
    " --LP64, --ILP64, --LLP64,\n"
    "   --ILP32, --LP32            set width of int, long and pointers\n"
    " --little-endian              allow little-endian word-byte conversions\n"
    " --big-endian                 allow big-endian word-byte conversions\n"
    " --unsigned-char              make \"char\" unsigned by default\n"
    " --show-parse-tree            show parse tree\n"
    " --show-symbol-table          show symbol table\n"
    " --show-goto-functions        show goto program\n"
    " --arch                       set architecture (default: "
                                   << configt::this_architecture() << ")\n"
    " --os                         set operating system (default: "
                                   << configt::this_operating_system() << ")\n"
    #ifdef _WIN32
    " --gcc                        use GCC as preprocessor\n"
    #endif
    " --no-arch                    don't set up an architecture\n"
    " --no-library                 disable built-in abstract C library\n"
    " --round-to-nearest           IEEE floating point rounding mode (default)\n"
    " --round-to-plus-inf          IEEE floating point rounding mode\n"
    " --round-to-minus-inf         IEEE floating point rounding mode\n"
    " --round-to-zero              IEEE floating point rounding mode\n"
    "\n"
    "Program instrumentation options:\n"
    " --bounds-check               enable array bounds checks\n"
    " --div-by-zero-check          enable division by zero checks\n"
    " --pointer-check              enable pointer checks\n"
    " --memory-leak-check          enable memory leak checks\n"
    " --signed-overflow-check      enable arithmetic over- and underflow checks\n"
    " --unsigned-overflow-check    enable arithmetic over- and underflow checks\n"
    " --nan-check                  check floating-point for NaN\n"
    " --error-label label          check that label is unreachable\n"
    " --show-properties            show the properties\n"
    " --no-assertions              ignore user assertions\n"
    " --no-assumptions             ignore user assumptions\n"
    "\n"
    "Other options:\n"
    " --version                    show version and exit\n"
    " --xml-ui                     use XML-formatted output\n"
    "\n";
}
