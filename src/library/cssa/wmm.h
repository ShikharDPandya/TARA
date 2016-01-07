/*
 * Copyright 2014, IST Austria
 *
 * This file is part of TARA.
 *
 * TARA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TARA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with TARA.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TARA_CSSA_WMM_H
#define TARA_CSSA_WMM_H

#include "constants.h"
#include "input/program.h"
#include "helpers/z3interf.h"
#include <vector>
#include <list>
#include "cssa/thread.h"
#include <unordered_map>
#include <unordered_set>
#include <boost/concept_check.hpp>

namespace tara{
namespace cssa {

  enum class event_kind_t { i, r, w };

//
// symbolic event
//
  class symbolic_event
  {
  public:
    symbolic_event( z3::context& ctx, hb_enc::encoding& hb_encoding,
                    unsigned _tid,
                    const variable& _v, const variable& _prog_v,
                    hb_enc::location_ptr loc, event_kind_t _et );

    // symbolic_event( z3::context& ctx, hb_enc::encoding& hb_encoding,
    //                 hb_enc::location_ptr loc, event_kind_t _et );

  public:
    unsigned tid;
    variable v;               // variable with ssa name
    variable prog_v;          // variable name in the program
    hb_enc::location_ptr loc; // location in program
    std::shared_ptr<tara::hb_enc::location> e_v; // variable for solver
    event_kind_t et;
    z3::expr guard;
    inline std::string name() const {
      return e_v->name;
    }
    friend std::ostream& operator<< (std::ostream& stream,
                                     const symbolic_event& var) {
      stream << var.name();
      return stream;
    }
    void debug_print(std::ostream& stream );
  };

  typedef std::shared_ptr<symbolic_event> se_ptr;

  inline se_ptr mk_se_ptr( z3::context& _ctx, hb_enc::encoding& _hb_enc,
                           unsigned _tid,
                           const variable& _v, const variable& _prog_v,
                           hb_enc::location_ptr _loc, event_kind_t _et ) {
    return std::make_shared<symbolic_event>(_ctx, _hb_enc, _tid, _v, _prog_v,
                                            _loc, _et);
  }

  // inline se_ptr mk_se_ptr( z3::context& _ctx,hb_enc::encoding& _hb_enc,
  //                          hb_enc::location_ptr _loc, event_kind_t _et ) {
  //   return std::make_shared<symbolic_event>( _ctx, _hb_enc, _loc, _et );
  // }


  struct se_hash {
    size_t operator () (const se_ptr &v) const {
      return std::hash<std::string>()(v->e_v->name);
    }
  };

  struct se_equal :
    std::binary_function <symbolic_event,symbolic_event,bool> {
    bool operator() (const se_ptr& x, const se_ptr& y) const {
      return std::equal_to<std::string>()(x->e_v->name, y->e_v->name);
    }
  };

  typedef std::unordered_set<se_ptr, se_hash, se_equal> se_set;

  typedef std::unordered_map<variable, se_set, variable_hash, variable_equal> var_to_ses_map;

  void debug_print_se_set(const se_set& set, std::ostream& out);

}}

#endif // TARA_CSSA_WMM_H
