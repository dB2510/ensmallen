/**
 * @file padam.hpp
 * @author Marcus Edel
 *
 * Definition of Partially adaptive momentum estimation method (Padam).
 *
 * ensmallen is free software; you may redistribute it and/or modify it under
 * the terms of the 3-clause BSD license.  You should have received a copy of
 * the 3-clause BSD license along with ensmallen.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef ENSMALLEN_PADAM_PADAM_HPP
#define ENSMALLEN_PADAM_PADAM_HPP

#include <ensmallen_bits/sgd/sgd.hpp>
#include "padam_update.hpp"

namespace ens {

/**
 * Partially adaptive momentum estimation method (Padam),
 * adopts historical gradient information to automatically adjust the
 * learning rate.
 *
 * For more information, see the following.
 *
 * @code
 * @article{
 *   title   = {Closing the Generalization Gap of Adaptive Gradient Methods in
 *              Training Deep Neural Networks},
 *   author  = {{Chen}, J. and {Gu}, Q.},
 *   journal = {ArXiv e-prints},
 *   url     = {https://arxiv.org/abs/1806.06763}
 *   year    = {2018}
 * }
 * @endcode
 *
 * Padam can optimize differentiable separable functions.  For more details, see
 * the documentation on function types include with this distribution or on the
 * ensmallen website.
 */
class Padam
{
 public:
  /**
   * Construct the Padam optimizer with the given function and parameters. The
   * defaults here are not necessarily good for the given problem, so it is
   * suggested that the values used be tailored to the task at hand.  The
   * maximum number of iterations refers to the maximum number of points that
   * are processed (i.e., one iteration equals one point; one iteration does not
   * equal one pass over the dataset).
   *
   * @param stepSize Step size for each iteration.
   * @param batchSize Number of points to process in a single step.
   * @param beta1 Exponential decay rate for the first moment estimates.
   * @param beta2 Exponential decay rate for the weighted infinity norm
   *        estimates.
   * @param partial Partially adaptive parameter.
   * @param epsilon Epsilon is the minimum allowed gradient.
   * @param maxIterations Maximum number of iterations allowed (0 means no
   *        limit).
   * @param tolerance Maximum absolute tolerance to terminate algorithm.
   * @param shuffle If true, the function order is shuffled; otherwise, each
   *        function is visited in linear order.
   * @param resetPolicy If true, parameters are reset before every Optimize
   *        call; otherwise, their values are retained.
   * @param exactObjective Calculate the exact objective (Default: estimate the
   *        final objective obtained on the last pass over the data).
   */
  Padam(const double stepSize = 0.001,
        const size_t batchSize = 32,
        const double beta1 = 0.9,
        const double beta2 = 0.999,
        const double partial = 0.25,
        const double epsilon = 1e-8,
        const size_t maxIterations = 100000,
        const double tolerance = 1e-5,
        const bool shuffle = true,
        const bool resetPolicy = true,
        const bool exactObjective = false) :
      optimizer(stepSize,
                batchSize,
                maxIterations,
                tolerance,
                shuffle,
                PadamUpdate(epsilon, beta1, beta2, partial),
                NoDecay(),
                resetPolicy,
                exactObjective)
  { /* Nothing to do here. */ }

  /**
   * Optimize the given function using Padam. The given starting point will be
   * modified to store the finishing point of the algorithm, and the final
   * objective value is returned.
   *
   * @tparam SeparableFunctionType Type of the function to optimize.
   * @tparam MatType Type of matrix to optimize with.
   * @tparam GradType Type of matrix to use to represent function gradients.
   * @tparam CallbackTypes Types of callback functions.
   * @param function Function to optimize.
   * @param iterate Starting point (will be modified).
   * @param callbacks Callback functions.
   * @return Objective value of the final point.
   */
  template<typename SeparableFunctionType,
           typename MatType,
           typename GradType,
           typename... CallbackTypes>
  typename MatType::elem_type Optimize(SeparableFunctionType& function,
                                       MatType& iterate,
                                       CallbackTypes&&... callbacks)
  {
    return optimizer.template Optimize<
        SeparableFunctionType, MatType, GradType, CallbackTypes...>(
        function, iterate, std::forward<CallbackTypes>(callbacks)...);
  }

  //! Forward the MatType as GradType.
  template<typename SeparableFunctionType,
           typename MatType,
           typename... CallbackTypes>
  typename MatType::elem_type Optimize(SeparableFunctionType& function,
                                       MatType& iterate,
                                       CallbackTypes&&... callbacks)
  {
    return Optimize<SeparableFunctionType, MatType, MatType,
        CallbackTypes...>(function, iterate,
        std::forward<CallbackTypes>(callbacks)...);
  }

  //! Get the step size.
  double StepSize() const { return optimizer.StepSize(); }
  //! Modify the step size.
  double& StepSize() { return optimizer.StepSize(); }

  //! Get the batch size.
  size_t BatchSize() const { return optimizer.BatchSize(); }
  //! Modify the batch size.
  size_t& BatchSize() { return optimizer.BatchSize(); }

  //! Get the smoothing parameter.
  double Beta1() const { return optimizer.UpdatePolicy().Beta1(); }
  //! Modify the smoothing parameter.
  double& Beta1() { return optimizer.UpdatePolicy().Beta1(); }

  //! Get the second moment coefficient.
  double Beta2() const { return optimizer.UpdatePolicy().Beta2(); }
  //! Modify the second moment coefficient.
  double& Beta2() { return optimizer.UpdatePolicy().Beta2(); }

  //! Get the partial adaptive parameter.
  double Partial() const { return optimizer.UpdatePolicy().Partial(); }
  //! Modify the partial adaptive parameter.
  double& Partial() { return optimizer.UpdatePolicy().Partial(); }

  //! Get the value used to initialise the mean squared gradient parameter.
  double Epsilon() const { return optimizer.UpdatePolicy().Epsilon(); }
  //! Modify the value used to initialise the mean squared gradient parameter.
  double& Epsilon() { return optimizer.UpdatePolicy().Epsilon(); }

  //! Get the maximum number of iterations (0 indicates no limit).
  size_t MaxIterations() const { return optimizer.MaxIterations(); }
  //! Modify the maximum number of iterations (0 indicates no limit).
  size_t& MaxIterations() { return optimizer.MaxIterations(); }

  //! Get the tolerance for termination.
  double Tolerance() const { return optimizer.Tolerance(); }
  //! Modify the tolerance for termination.
  double& Tolerance() { return optimizer.Tolerance(); }

  //! Get whether or not the individual functions are shuffled.
  bool Shuffle() const { return optimizer.Shuffle(); }
  //! Modify whether or not the individual functions are shuffled.
  bool& Shuffle() { return optimizer.Shuffle(); }

  //! Get whether or not the actual objective is calculated.
  bool ExactObjective() const { return optimizer.ExactObjective(); }
  //! Modify whether or not the actual objective is calculated.
  bool& ExactObjective() { return optimizer.ExactObjective(); }

  //! Get whether or not the update policy parameters
  //! are reset before Optimize call.
  bool ResetPolicy() const { return optimizer.ResetPolicy(); }
  //! Modify whether or not the update policy parameters
  //! are reset before Optimize call.
  bool& ResetPolicy() { return optimizer.ResetPolicy(); }

 private:
  //! The Stochastic Gradient Descent object with Padam policy.
  SGD<PadamUpdate> optimizer;
};

} // namespace ens

#endif
