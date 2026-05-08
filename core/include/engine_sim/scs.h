#ifndef ATG_ENGINE_SIM_SCS_H
#define ATG_ENGINE_SIM_SCS_H

// Re-export all SCS headers
#include "generic_rigid_body_system.h"
#include "optimized_nsv_rigid_body_system.h"

#include "euler_ode_solver.h"
#include "rk4_ode_solver.h"
#include "nsv_ode_solver.h"

#include "gauss_seidel_sle_solver.h"
#include "gaussian_elimination_sle_solver.h"
#include "conjugate_gradient_sle_solver.h"

#include "static_force_generator.h"
#include "gravity_force_generator.h"
#include "spring.h"

#include "rigid_body_system.h"
#include "rigid_body.h"
#include "system_state.h"

#include "fixed_position_constraint.h"
#include "line_constraint.h"
#include "link_constraint.h"
#include "clutch_constraint.h"
#include "rotation_friction_constraint.h"
#include "rolling_constraint.h"
#include "constant_rotation_constraint.h"
#include "constant_speed_motor.h"

#include "sle_solver.h"
#include "matrix.h"
#include "sparse_matrix.h"

#include "force_generator.h"
#include "ode_solver.h"
#include "constraint.h"
#include "utilities.h"

#endif /* ATG_ENGINE_SIM_SCS_H */