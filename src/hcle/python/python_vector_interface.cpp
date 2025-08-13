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
        .def(py::init<int, std::string, std::string, std::string, uint8_t, uint8_t, uint8_t, bool, bool, uint8_t>(),
             py::arg("num_envs"),
             py::arg("rom_path"),
             py::arg("game_name"),
             py::arg("render_mode"),
             py::arg("obs_height"),
             py::arg("obs_width"),
             py::arg("frame_skip"),
             py::arg("maxpool"),
             py::arg("grayscale"),
             py::arg("stack_num"))

        .def("get_action_space_size", &hcle::environment::HCLEVectorEnvironment::getActionSpaceSize)

        .def("reset", [](hcle::environment::HCLEVectorEnvironment &self)
             {
            const std::vector<py::ssize_t> obs_shape = {self.getNumEnvs(), 240, 256, 3};
            auto obs = py::array_t<uint8_t>(obs_shape);
            
            // Release the GIL to allow C++ threads to run
            py::gil_scoped_release release;
            self.reset(obs.mutable_data());
            py::gil_scoped_acquire acquire;

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
            
            // CHANGE 1: The 'dones' array should be uint8_t to match your C++ function
            auto dones = py::array_t<uint8_t>(self.getNumEnvs());

            std::vector<uint8_t> actions_vec(actions.data(), actions.data() + actions.size());

            // Release the GIL to allow C++ threads to run
            py::gil_scoped_release release;

            // CHANGE 2: Replace the two separate calls with a single call to your new step function
            self.step(actions_vec, 
                    obs.mutable_data(), 
                    rewards.mutable_data(), 
                    dones.mutable_data());
                    
            py::gil_scoped_acquire acquire;

            // Return the populated NumPy arrays
            return py::make_tuple(obs, rewards, dones); })

        .def("step_async", [](hcle::environment::HCLEVectorEnvironment &self, py::array_t<uint8_t> actions)
             {
                 if (actions.ndim() != 1 || actions.shape(0) != self.getNumEnvs())
                 {
                     throw std::runtime_error("Actions must be a 1D numpy array with size equal to num_envs.");
                 }

                 // Convert numpy array to C++ vector to pass to the C++ send method
                 std::vector<uint8_t> actions_vec(actions.data(), actions.data() + actions.size());

                 py::gil_scoped_release release;
                 self.step_async(actions_vec);
                 py::gil_scoped_acquire acquire;
                 // send has a void return type, so we return nothing.
             })

        .def("step_wait", [](hcle::environment::HCLEVectorEnvironment &self)
             {
            // Prepare the output NumPy arrays that our C++ function will fill
            const std::vector<py::ssize_t> obs_shape = {self.getNumEnvs(), 240, 256, 3};
            auto obs = py::array_t<uint8_t>(obs_shape);
            auto rewards = py::array_t<float>(self.getNumEnvs());
            auto dones = py::array_t<uint8_t>(self.getNumEnvs());

            py::gil_scoped_release release;
            // Call the C++ recv method, passing pointers to the NumPy buffers
            self.step_wait(obs.mutable_data(), rewards.mutable_data(), dones.mutable_data());
            py::gil_scoped_acquire acquire;

            // Return the populated NumPy arrays as a tuple
            return py::make_tuple(obs, rewards, dones); });
}