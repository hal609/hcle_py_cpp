#include <map>
#include <string>
#include <memory>
#include <stdexcept>

#include "game_logic.hpp"
#include "smb1.hpp"
#include "kungfu.hpp"
#include "tetris.hpp"
#include "baseball.hpp"
#include "drmario.hpp"
#include "excitebike.hpp"
#include "golf.hpp"
#include "lolo1.hpp"
#include "mariobros.hpp"
#include "mtpo.hpp"
#include "smb2.hpp"
#include "smb3.hpp"
#include "tmnt.hpp"
#include "zelda1.hpp"

namespace fs = std::filesystem;

static const std::map<std::string, std::string> EXPECTED_ROM_HASHES = {
    {"smb1.bin", "84b3f808f23348638b43f4a3bce225f1"},
    {"kungfu.bin", "bb16caf5223c21a48f4b5f4986861922"},
    {"tetris.bin", "ec58574d96bee8c8927884ae6e7a2508"},
    {"baseball.bin", "131e911f248cf42313184d3dbbd576db"},
    {"drmario.bin", "d3ec44424b5ac1a4dc77709829f721c9"},
    {"excitebike.bin", "d7fe15cf2bc7b6582c07d12b3cf3bede"},
    {"golf.bin", "a8ef965eabfb57c59a9a6754a5581d77"},
    {"lolo1.bin", "38516649d5d9c0b51a9a578c8178ee5b"},
    {"mariobro.bin", "d85e4dbfb52687c83915ac3e4cc08bbb"},
    {"mtpo.bin", "b9a66b2760daa7d5639cbad903de8a18"},
    {"smb2.bin", "71576d8339bd63198fcfc51a92016d58"},
    {"smb3.bin", "bb5c4b6d4d78c101f94bdb360af502f3"},
    {"tmnt.bin", "5e24ccd733d15e42d847274e7add0a76"},
    {"zelda1.bin", "337bd6f1a1163df31bf2633665589ab0"}};

namespace hcle
{
   using GameLogicPtr = std::unique_ptr<games::GameLogic>;
   using GameLogicMap = std::map<std::string, GameLogicPtr>;

   static GameLogicMap initialize_game_logic_map()
   {
      GameLogicMap logic_map;
      logic_map["smb1"] = std::make_unique<games::SMB1Logic>();
      logic_map["kungfu"] = std::make_unique<games::KungFuLogic>();
      logic_map["tetris"] = std::make_unique<games::TetrisLogic>();
      logic_map["baseball"] = std::make_unique<games::BaseballLogic>();
      logic_map["drmario"] = std::make_unique<games::DrMarioLogic>();
      logic_map["excitebike"] = std::make_unique<games::ExcitebikeLogic>();
      logic_map["golf"] = std::make_unique<games::GolfLogic>();
      logic_map["lolo1"] = std::make_unique<games::Lolo1Logic>();
      logic_map["mariobros"] = std::make_unique<games::MarioBrosLogic>();
      logic_map["mtpo"] = std::make_unique<games::MTPO_Logic>();
      logic_map["smb2"] = std::make_unique<games::SMB2Logic>();
      logic_map["smb3"] = std::make_unique<games::SMB3Logic>();
      logic_map["tmnt"] = std::make_unique<games::TMNTLogic>();
      logic_map["zelda1"] = std::make_unique<games::Zelda1Logic>();

      return logic_map;
   }

   static GameLogicMap game_logic_map = initialize_game_logic_map();

   inline games::GameLogic *get_game_logic(const std::string &game_name)
   {
      try
      {
         return game_logic_map.at(game_name).get();
      }
      catch (const std::out_of_range &e)
      {
         return nullptr;
      }
   }
   inline std::string get_rom_path(const std::string &game_name)
   {
      fs::path data_dir;

      // Check for rom dir environment variable
      const char *env_path = std::getenv("HCLE_ROMS_DIR");
      if (env_path)
      {
         data_dir = fs::path(env_path);
         std::cout << "Loading roms from " << fs::absolute(data_dir) << "..." << std::endl;
      }
      else
      {
         // !!! TEMP -> CHANGE BEFORE RELEASE !!!
         // Fallback: Hardcoded path to ROMs
         // !!! TEMP -> CHANGE BEFORE RELEASE !!!
         data_dir = "C:\\Users\\offan\\Documents\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms";
      }

      // Check dir exists
      if (!fs::exists(data_dir))
      {
         throw std::runtime_error("ROM directory does not exist: " + data_dir.string());
      }
      if (!fs::is_directory(data_dir))
      {
         throw std::runtime_error("ROM path is not a directory: " + data_dir.string());
      }

      std::string bin_file = game_name + ".bin";
      fs::path bin_path = data_dir / bin_file;

      if (EXPECTED_ROM_HASHES.find(bin_file) == EXPECTED_ROM_HASHES.end())
      {
         std::cerr << "Warning: ROM " << game_name << " is not officially supported." << std::endl;
         return "";
      }

      if (!fs::exists(bin_path))
      {
         throw std::runtime_error("ROM file not found at path: " + bin_path.string());
      }

      return bin_path.string();
   }

}