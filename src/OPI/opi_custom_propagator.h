#ifndef OPI_HOST_MODULED_PROPAGATOR_H
#define OPI_HOST_MODULED_PROPAGATOR_H

#include "opi_propagator.h"
#include <vector>
namespace OPI
{
	class PerturbationModule;
	class PropagatorIntegrator;

	//! \brief This class represents a propagator which can be composed from different perturbation modules and an integrator at runtime.
	//! \ingroup CPP_API_GROUP
	class CustomPropagator:
			public Propagator
	{
		public:
			//! Creates a new custom propagator with the specified name
			CustomPropagator(const std::string& name);
			~CustomPropagator();
			/// Adds a module to this propagator
			void addModule(PerturbationModule* module);
			/// Sets the integrator for this propagator
			void setIntegrator(PropagatorIntegrator* integrator);

		protected:
			/// Override the propagation method
			virtual ErrorCode runPropagation(ObjectData& data, float years, float seconds, float dt );

		private:
			std::vector<PerturbationModule*> modules;
			PropagatorIntegrator* integrator;

	};
}

#endif
