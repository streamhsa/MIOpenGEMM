/*******************************************************************************
 * Copyright (C) 2017 Advanced Micro Devices, Inc. All rights reserved.
 *******************************************************************************/

#ifndef GUARD_MIOPENGEMM_PROGRAMCACHER_HPP
#define GUARD_MIOPENGEMM_PROGRAMCACHER_HPP

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>
#include <miopengemm/geometry.hpp>
#include <miopengemm/hyperparams.hpp>
#include <miopengemm/kernelstring.hpp>
#include <miopengemm/oclutil.hpp>
#include <miopengemm/outputwriter.hpp>
#include <miopengemm/platform.hpp>
#include <miopengemm/programs.hpp>

namespace MIOpenGEMM
{

enum BetaType
{
  IsZero,
  IsOne,
  IsOther
};

enum UnusualGemmId
{
  ZeroOperation = -1,
  ZeroKandBeta  = -2
};

template <typename T>
BetaType get_beta_type(T beta)
{
  if (beta >= T(0) && beta <= T(0))
  {
    return BetaType::IsZero;
  }

  else if (beta >= T(1) && beta <= T(1))
  {
    return BetaType::IsOne;
  }

  return BetaType::IsOther;
}

class ProgramCacher
{

  private:
  constexpr static size_t max_cache_size = 20000;

  size_t current_ID = 0;

  public:
  // std::array<Programs, max_cache_size> program_cache;  // 7MB @ max_cache_size = 10000.
  // std::array<HyPas, max_cache_size>    hyper_params;

  std::array<std::unique_ptr<std::pair<Programs, HyPas>>, max_cache_size> prohyp_cache;

  std::unordered_map<std::string, int> IDs;
  std::mutex mutt;

  int get_ID(bool                isColMajor,
             bool                tA,
             bool                tB,
             bool                tC,
             size_t              m,
             size_t              n,
             size_t              k,
             size_t              lda,
             size_t              ldb,
             size_t              ldc,
             std::vector<size_t> w_size,
             BetaType            beta_type,
             char                floattype,
             cl_command_queue*   ptr_queue);

  int get_ID_from_geom(const Geometry& gg, BetaType beta, cl_command_queue* ptr_queue);
};

ProgramCacher& get_cacher();
}

#endif
