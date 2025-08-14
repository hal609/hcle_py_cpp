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

        .def("get_action_space_size", [](hcle::environment::HCLEVectorEnvironment &self)
             { return self.getActionSet().size(); })
        .def("reset", [](hcle::environment::HCLEVectorEnvironment &self)
             {
                py::gil_scoped_release release;
                auto timesteps = self.reset();
                py::gil_scoped_acquire acquire;

                const int batch_size = timesteps.size();
                const auto shape_info = self.get_observation_shape();
                const int stack_num = std::get<0>(shape_info);
                const int height = std::get<1>(shape_info);
                const int width = std::get<2>(shape_info);
                const int channels = std::get<3>(shape_info); // Assuming 3 for RGB, 1 for Grayscale
                const bool is_grayscale = (channels == 1);

                // 2. Create the output NumPy arrays with the correct shapes and data types.
                py::array_t<uint8_t> obs_np;
                if (is_grayscale) {
                    obs_np = py::array_t<uint8_t>({batch_size, stack_num, height, width});
                } else {
                    obs_np = py::array_t<uint8_t>({batch_size, stack_num, height, width, channels});
                }
                // py::array_t<float> rewards_np(batch_size);
                // py::array_t<bool> dones_np(batch_size);
                py::array_t<bool> env_ids_np(batch_size);

                // 3. Get direct, mutable pointers to the underlying data buffers of the NumPy arrays.
                auto obs_ptr = static_cast<uint8_t*>(obs_np.mutable_data());
                // auto rewards_ptr = static_cast<float*>(rewards_np.mutable_data());
                // auto dones_ptr = static_cast<bool*>(dones_np.mutable_data());
                auto env_ids_ptr = static_cast<bool*>(env_ids_np.mutable_data());

                // 4. Loop through the C++ Timestep vector and copy the data into the NumPy arrays.
                const size_t single_obs_size = stack_num * height * width * channels;
                for (int i = 0; i < batch_size; ++i) {
                    const auto& timestep = timesteps[i];

                    // Copy the observation pixels.
                    if (timestep.observation.size() == single_obs_size) {
                        std::memcpy(obs_ptr + i * single_obs_size, timestep.observation.data(), single_obs_size);
                    } else {
                        throw std::runtime_error("C++ observation size does not match expected NumPy shape.");
                    }

                    // Copy the scalar reward and done values.
                    env_ids_ptr[i] = timestep.env_id;
                    // rewards_ptr[i] = timestep.reward;
                    // dones_ptr[i] = timestep.done;
                }

                // Create info dict
                py::dict info;
                info["env_id"] = env_ids_np;

                return py::make_tuple(obs_np, info); })
        .def("step_async", [](hcle::environment::HCLEVectorEnvironment &self, py::array_t<uint8_t> actions)
             {
                py::buffer_info actions_buf = actions.request();
                auto *actions_ptr = static_cast<uint8_t *>(actions_buf.ptr);
                std::vector<int> actions_vec(actions_ptr, actions_ptr + actions.size());

                py::gil_scoped_release release;
                self.send(actions_vec);
                py::gil_scoped_acquire acquire; })

        .def("step_wait", [](hcle::environment::HCLEVectorEnvironment &self)
             {
                std::vector<hcle::vector::Timestep> timesteps = self.recv();
                py::gil_scoped_acquire acquire;

                const int batch_size = timesteps.size();
                const auto shape_info = self.get_observation_shape();
                const int stack_num = std::get<0>(shape_info);
                const int height = std::get<1>(shape_info);
                const int width = std::get<2>(shape_info);
                const int channels = std::get<3>(shape_info); // Assuming 3 for RGB, 1 for Grayscale
                const bool is_grayscale = (channels == 1);

                // 2. Create the output NumPy arrays with the correct shapes and data types.
                py::array_t<uint8_t> obs_np;
                if (is_grayscale)
                {
                    obs_np = py::array_t<uint8_t>({batch_size, stack_num, height, width});
                }
                else
                {
                    obs_np = py::array_t<uint8_t>({batch_size, stack_num, height, width, channels});
                }
                py::array_t<float> rewards_np(batch_size);
                py::array_t<bool> dones_np(batch_size);

                // 3. Get direct, mutable pointers to the underlying data buffers of the NumPy arrays.
                auto obs_ptr = static_cast<uint8_t *>(obs_np.mutable_data());
                auto rewards_ptr = static_cast<float *>(rewards_np.mutable_data());
                auto dones_ptr = static_cast<bool *>(dones_np.mutable_data());

                // 4. Loop through the C++ Timestep vector and copy the data into the NumPy arrays.
                const size_t single_obs_size = stack_num * height * width * channels;
                for (int i = 0; i < batch_size; ++i)
                {
                    const auto &timestep = timesteps[i];

                    // Copy the observation pixels.
                    if (timestep.observation.size() == single_obs_size)
                    {
                        std::memcpy(obs_ptr + i * single_obs_size, timestep.observation.data(), single_obs_size);
                    }
                    else
                    {
                        throw std::runtime_error("C++ observation size does not match expected NumPy shape.");
                    }

                    // Copy the scalar reward and done values.
                    rewards_ptr[i] = timestep.reward;
                    dones_ptr[i] = timestep.done;
                }
                return py::make_tuple(obs_np, rewards_np, dones_np); });
}