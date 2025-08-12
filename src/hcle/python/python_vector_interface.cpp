#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "hcle/environment/hcle_vector_environment.hpp"

#include <iostream> // Required for std::cout
#include <vector>   // Required for std::vector

namespace py = pybind11;

void init_vector_bindings(py::module_ &m)
{
    py::class_<hcle::environment::HCLEVectorEnvironment>(m, "HCLEVectorEnvironment")
        // The type arguments here MUST match the C++ constructor's order:
        // int, string, string
        .def(py::init<int, std::string, std::string, std::string>(),
             // The py::arg names must ALSO be in the same order.
             py::arg("num_envs"),
             py::arg("rom_path"),
             py::arg("render_mode"))

        .def("get_action_space_size", &hcle::environment::HCLEVectorEnvironment::getActionSpaceSize)

        .def("reset", [](hcle::environment::HCLEVectorEnvironment &self)
             {
            // Create the NumPy array that will hold the observations
            const std::vector<py::ssize_t> obs_shape = {self.getNumEnvs(), 240, 256, 3};
            auto obs = py::array_t<uint8_t>(obs_shape);
            
            // Release the GIL to allow C++ threads to run
            py::gil_scoped_release release;
            // Call the C++ reset function, passing the NumPy buffer's pointer
            self.reset(obs.mutable_data());
            py::gil_scoped_acquire acquire;

            // Return the populated NumPy array
            return obs; })

        .def("step", [](hcle::environment::HCLEVectorEnvironment &self, py::array_t<uint8_t> actions)
             {
            if (actions.ndim() != 1 || actions.shape(0) != self.getNumEnvs()) {
                throw std::runtime_error("Actions must be a 1D numpy array with size equal to num_envs.");
            }
            
            // Prepare output NumPy arrays
            const std::vector<py::ssize_t> obs_shape = {self.getNumEnvs(), 240, 256, 3};
            auto obs = py::array_t<uint8_t>(obs_shape);
            auto rewards = py::array_t<float>(self.getNumEnvs());
            auto dones = py::array_t<bool>(self.getNumEnvs());

            std::vector<uint8_t> actions_vec(actions.data(), actions.data() + actions.size());

            // Release the GIL to allow C++ threads to run
            py::gil_scoped_release release;
            // Call the C++ step function, passing pointers to the NumPy buffers
            self.step(actions_vec, obs.mutable_data(), rewards.mutable_data(), dones.mutable_data());
            py::gil_scoped_acquire acquire;

            // Return the populated NumPy arrays
            return py::make_tuple(obs, rewards, dones); });
}