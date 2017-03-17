#include <tinygemm/basegenerator.hpp>

#include <sstream>
#include <chrono>
#include <sstream>

namespace tinygemm{
namespace basegen{

      
BaseGenerator::BaseGenerator(const tinygemm::hyperparams::HyperParams & hp_, const tinygemm::TinyGemmGeometry & gg_, const tinygemm::derivedparams::DerivedParams & dp_, const std::string & type_): hp(hp_), gg(gg_), dp(dp_), type(type_), kernelname("tg_" + type_) {

  //kernelname = get_generic_kernelname(type);
}
  

void BaseGenerator::append_parameter_list_from_usage(std::stringstream & ss){
  
  char first_char = '\n';
  
  ss << "\n(";
  if (uses_a == true){
    ss << first_char << "__global const TFLOAT * restrict a, \n" << "const unsigned a_offset";
    first_char = ',';
  }

  if (uses_b == true){
    ss << first_char << "\n__global const TFLOAT * restrict b, \n" << "const unsigned b_offset";
    first_char = ',';
  }
  
  if (uses_c == true){
    ss << first_char << "\n__global TFLOAT       *          c, \n" << "const unsigned c_offset";
    first_char = ',';
  }

  if (uses_workspace == true){
    
    //if using c, workspace is const. this is a bit hacky, might have a kernel which uses c and modifies w too. 
    if (uses_c == true){
      ss << first_char << "\n__global const TFLOAT * restrict w,\n";
    }
    else{
      ss << first_char << "\n__global TFLOAT * restrict w,\n";
    }
    ss << "const unsigned w_offset";
    first_char = ',';
  }


  if (uses_alpha == true){
    ss << first_char << "\nconst TFLOAT alpha";
    first_char = ',';
  }
  
  
  if (uses_beta == true){
    ss << first_char << "\nconst TFLOAT beta";
    first_char = ',';
  }
  
  ss << ")\n";  
  
}




void BaseGenerator::append_stride_definitions(char x, std::stringstream & ss, unsigned workspace_type, bool withcomments, std::string macro_prefix, bool with_x_in_name){
  if (withcomments) ss << "/* strides parallel to k (unroll) in " << x << ". MACRO_STRIDE_" << x << " is between unroll tiles, STRIDE_" << x << " is within unroll tiles  */\n"; 
  
  std::string x_bit = with_x_in_name ? "_" + std::string(1, x) : "";
  for (std::string orth :  {"PLL", "PERP"}){
    bool pll_k = ("PLL" == orth);
    ss << "#define " << macro_prefix << "STRIDE_" << orth << "_K" << x_bit << " " << dp.get_stride(x, pll_k, false, workspace_type) << "\n";
    ss << "#define " << macro_prefix << "MACRO_STRIDE_" << orth << "_K" << x_bit << " " << dp.get_stride(x, pll_k, true, workspace_type) << "\n";
  }
}


void BaseGenerator::append_unroll_block_geometry(char x, std::stringstream & ss, bool withcomments, bool with_x_string){

  std::string x_string = with_x_string ? "_" + std::string(1, x) : "";
  x = (x == 'a' ? 'A' : (x == 'b' ? 'B' : x));
  
  ss << "\n";
  if (withcomments) ss << "/* macro tiles define the pattern of C that workgroups (threads with shared local memory) process */\n";
  ss << "#define " << "MACRO_TILE_LENGTH" << x_string << " " << hp.at(x).macro_tile_length.val << "\n";

  if (withcomments) ss << "/* number of elements in load block : MACRO_TILE_LENGTH" << x_string << " * UNROLL */\n";
  ss << "#define " << "N_ELEMENTS_IN" << x_string << "_UNROLL "<< dp.at(x).n_elements_in_unroll <<"\n";

  if (withcomments) {
    ss << "/* number of groups covering " << (x == 'A' ? 'M': 'N') <<  " / MACRO_TILE_LENGTH" << x_string;
    if (dp.main_use_edge_trick == 1){
      ss << " + (PRESHIFT_FINAL_TILE" << x_string << " != MACRO_TILE_LENGTH" << x_string << ")";
    }
    ss << " */" << "\n";
  }
  ss << "#define " << "N_GROUPS" << x_string << " " <<  dp.at(x).n_groups << "\n";

  if (dp.main_use_edge_trick != 0){
    if (withcomments) ss <<  "/* 1 + ("  << (x == 'A' ? 'M': 'N') << " - 1) % MACRO_TILE_LENGTH" << x_string << ". somewhere in 1 ... MACRO_TILE_LENGTH" << x_string << "  */ \n";
    ss << "#define " << "PRESHIFT_FINAL_TILE" << x_string << " " << dp.at(x).preshift_final_tile << "\n";
  }
}



std::string BaseGenerator::get_time_string(){//  

  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  std::time_t generation_time = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss <<  
R"(/* ****************************************************************************
* This )" << type << " kernel string was generated on " << std::ctime(&generation_time) << 
R"(**************************************************************************** */)";
  return ss.str();
}

std::string BaseGenerator::get_what_string(){
  return R"(/* ***********************************************
* These parameters define WHAT this kernel does *
* *********************************************** */)";
}

std::string BaseGenerator::get_how_string(){
  return R"(/* ****************************************
 * These parameters define HOW it does it *
 * *****************************************/)";
}

std::string BaseGenerator::get_derived_string(){
  return R"(/* *****************************************************************************
 * The following are all implied by the preceding: these are NOT free parameters!
 * **************************************************************************** */)";
}




}
}


