#pragma once

#include "Engine/Core.h"
#include "Engine/Utilities/Log.h"
#include "json.h"
#include <string>
#include <filesystem>
#include <set>

//Written by Kevin Vilanova, KevinAlexV.
//Class is intended to import and manage assets and resources used by the game engine.
//Documentation for the stb_image library can be found here: https://github.com/nothings/stb/blob/master/stb_image.h | https://gist.github.com/roxlu/3077861
//For texture loading, the expectation is to bind a texture then render any objects using that texture before switching to another texture, as opposed to doing a load 
//per use-case (ie; do bind text 1 -> draw object A B C, then bind text 2).
struct GLFWwindow;

namespace Engine
{
	typedef unsigned int GLuint;

	class ENGINE_API ResourceManager
	{

	public:
		#pragma region Structs
		//A struct to contain all data contained within an STB image. 
		struct ImageData
		{
			unsigned char* image{ nullptr };
			int width{ 0 }, height{ 0 }, numComponents{ 0 };
			GLuint texID{ 0 };
		};

		//A struct to contain sprite sheet data (sprite dimensions, GL texture ID, etc).
		struct SpriteSheet
		{
			float spriteWidth{ 0.f }, spriteHeight{ 0.f }, texWidthFraction{ 0.f }, texHeightFraction{ 0.f };
			int spriteSheetWidth{ 0 }, spriteSheetHeight{ 0 }, numSprites{ 0 }, spritesPerRow{ 3 }, spritesPerColumn{ 3 };

			GLuint texID{ 0 };
		};
		#pragma endregion
		
		#pragma region Singleton Instance Management

		//Singletons should not be cloneable, this is to prevent clones.
		ResourceManager(ResourceManager& other) = delete;

		//Singletons should not be assignable, this is to prevent that.
		void operator=(const ResourceManager&) = delete;

		//This retrieves a pointer to the current instance of ResourceManager. If it doesn't exist, then one will be created and returned.
		static ResourceManager* getInstance();

		#pragma endregion

		#pragma region Get & Set Functions
		//Function to save provided json data to a provided file. Will save to "Assets/" directory if path is not specified.
		void saveJsonFile(nlohmann::json data, std::string fileName,std::string extension = ".json", std::string path = "Assets/");

		//Function to set icon for the application.
		void setAppIcon(std::string& appIcon, GLFWwindow* window);

		//Function to return a json (formatted as jsons from nlohmanns library) from the hashmap based on a provided name. Returns a nullptr if no json is found.
		nlohmann::json getJsonData(const std::string& name);

		//Function to return Image data from texture stored at path name. Returns empty ImageData if file not found.
		ImageData getTexture(const std::string& name);

		//Function to return File path for provided font name. Returns empty string if path not found.
		std::string getFile(const std::string& name);

		//Based on the provided spritesheet filename, return a SpriteSheet struct (with info on a spriteSheet) stored within a map, or load it from the system. Returns an empty SpriteSheet if not found. 
		SpriteSheet getSpritesheet(const std::string& name, float spriteWidth = 0.f, float spriteHeight = 0.f);

		//Function to return a shader (formatted as a string with newlines to seperate GSLS code) from the hashmap based on a provided name. Returns an empty string if no shader is found.
		std::string getShaderData(const std::string& name);
		#pragma endregion

	protected:
		#pragma region Constructors & Destructors

		//Constructor for ResourceManager Class. Finds all files under the 'data' path and stores the paths in a map. 
		ResourceManager();

		//Destructor
		~ResourceManager();
		#pragma endregion

	private:

		//The static instance of ResourceManager that will be returned if the resource manager is requested.
		static ResourceManager* m_pinstance;

		//static lock m_mutex is the lock to be used for static functions. Otherwise, m_functionlock should be utilized.
		static std::mutex m_mutex;
		std::mutex m_functionLock;

		int m_RGB { 3 }, m_RGBA { 4 };

		const std::string m_appAssetsPath{ "Assets" };
		const std::string m_engineAssetsPath{"EngineAssets"};
			
		#pragma region File Extension Variables
			
		//Extensions for all files handled through resource manager.
		std::set<std::string> m_jsonFileExts { "json", "bda"};
		
		std::vector<std::string> m_shaderFileExts { "vs", "fs", "as"};
			
		std::vector<std::string> m_imageFileExts { "jpeg", "png", "jpg"};
		#pragma endregion
		
		#pragma region File Storage Variables
										
		//Every file path found under the specified resources folder, 'm_sourcePath'.
		std::unordered_map<std::string, std::string> m_filePaths{};
		
		//Map to store each processed texture containing the texture ID and the texture data.
		std::unordered_map<std::string, ImageData> m_textures{};

		//Map to store each processed spritesheet containing data on the sheet itself, and the respective texture ID.
		std::unordered_map<std::string, SpriteSheet> m_spritesheetsTex{};
		
		//A map to store Json files, utilizing the json library.
		std::unordered_map<std::string, nlohmann::json> m_jsons{};
			
		//A map to store shader files as strings
		std::unordered_map<std::string, std::string> m_shaders{};

		//Vector containing all source paths to search for files (think of these as the 'root' asset folders). 
		//This can be added to from Application/Game code for custom paths.
		std::vector<std::string> m_sourcePaths{ std::filesystem::current_path().string() + "\\" + m_appAssetsPath,
													std::filesystem::current_path().parent_path().string() + "\\" + m_appAssetsPath,
													std::filesystem::current_path().parent_path().string() + "\\Engine\\" + m_engineAssetsPath,
													std::filesystem::current_path().parent_path().string() + "\\" + m_engineAssetsPath };
		#pragma endregion
			
		#pragma region Set & Read/Load Functions

		//Function to save all file paths into m_filePaths within the provided path, including subdirectories.
		void saveFilePaths(const std::string& path);

		//Add file path from provided directory_entry to the m_filePaths map.
		void saveFilePath(std::filesystem::directory_entry path);

		//Generate a new texture with the provided image data, and return the ID for the texture.
		GLuint generateTexture(ImageData& img);

		//Function to return an image (formatted as a pointer to an unsigned char) from the hashmap based on a provided name. Returns a nullptr if no image is found.
		//NOTE: After using the image, you MUST use stbi_image_free(); in order to free the memory of the image.
		ImageData readImageData(const std::string& name);
		
		//Read file found at sourcePath and return it as a string. Will only read the provided string. Returns an empty string if nothing is found.
		std::string readSource(const std::string &sourcePath);
		#pragma endregion
	};
}