#include "ResourceManager.h"

namespace Engine
{
	
	//Finds all files under the 'data' path and stores the paths in a map. 
	ResourceManager::ResourceManager() 
	{
		//Store all files listed under m_sourcePaths in the m_filePaths map
		for (int e = 0; e < m_sourcePaths.size(); e++)
		{
			std::string path = m_sourcePaths[e];

			saveFilePaths(path);
		}
	}

#pragma region Set Functions
	
	//Save all file paths from the provided path into m_filePaths, including subdirectories.
	void ResourceManager::saveFilePaths(const std::string& path)
	{
		//Using a recursive iterator, check each path under the root paths (m_sourcePaths) and add the file path to m_filePaths.
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
		{
			//If the file is not a directory, store the path.
			if (!std::filesystem::is_directory(entry.path()))
			{
				saveFilePath(entry);
			}
		}
	}

	//Load a file from a path into the m_filePaths map, if it is not already in the map and is a file.
	void ResourceManager::saveFilePath(std::filesystem::directory_entry path)
	{
		//File path is the full path of the found folder or file. Filename is the folder/file name, and file extension should contain the file type (to differentiate between file and folder).
		std::filesystem::path filePath = path.path();
		std::string fileName = filePath.filename().string();

		//Check if file exists in map.
		auto it = m_filePaths.find(fileName);
		
		//If the file name is not already in the map add it to the map.
		if (it == m_filePaths.end())
		{
			m_filePaths.insert(std::pair<std::string, std::string>(fileName, filePath.string()));
			GE_CORE_INFO("[ResourceManager] File added to m_filePaths [{0}]", filePath.string());
		}
	}
#pragma endregion
	
#pragma region Get Functions
	//Based on the provided filename, return the json file from a map, or load it from the system.
	nlohmann::json ResourceManager::getJsonData(const std::string& Name)
	{
		//Initilaze the return value, and create a map iterator to check if the json is already loaded.
		nlohmann::json data = nullptr;
		std::unordered_map<std::string, nlohmann::json>::iterator it = m_jsons.find(Name);
		
		//If loaded, return the json. Else, load and store the json, then return it.
		if (it != m_jsons.end())
		{
			//Get value based on map key "Name"
			data = m_jsons.find(Name)->second;
			
			GE_CORE_WARN("[ResourceManager] Previously loaded " + Name + " found.");
			return data;
		}
		
		//Create an iterator to check if the file exists in the map (done since using m_filePaths[Name] will create a new entry if it doesn't exist).
		std::unordered_map<std::string, std::string>::iterator jsonPath = m_filePaths.find(Name);
			
		//If json path is found (should be loaded into m_filePaths, currently upon initialization), load, store, then return the json.
		if (jsonPath != m_filePaths.end())
		{
			std::string path = m_filePaths[Name];
			std::string extension = path.substr(path.find_last_of('.') + 1);
			
			//Ensure the file is a json file
			if (extension == "json")
			{
				GE_CORE_INFO("[ResourceManager] " + Name + " found.");
				
				data = nlohmann::json::parse(readSource(path));

				m_jsons.insert(std::pair<std::string, nlohmann::json>(Name, data));
				return data;
			}
		}

		GE_CORE_INFO("[ResourceManager] " + Name + " not found.");
		return nullptr;
	}

	//Based on the provided filename, return the shader source from a map, or load it from the system.
	std::string ResourceManager::getShaderData(const std::string& Name)
	{
		//Initilaze the return value, and create a map iterator to check if the shader is already loaded.
		std::string data;
		std::unordered_map<std::string, std::string>::iterator it = m_shaders.find(Name);
		
		//If loaded, return the shader. Else, load and store the shader, then return it.
		if (it != m_shaders.end())
		{
			//Get value based on map key "Name"
			data = m_shaders.find(Name)->second;

			GE_CORE_WARN("[ResourceManager] Previously loaded " + Name + " found.");
			return data;
		}
		
		//Create an iterator to check if the file exists in the map (done since using m_filePaths[Name] will create a new entry if it doesn't exist).
		std::unordered_map<std::string, std::string>::iterator shaderPath = m_filePaths.find(Name);
			
		//If shader path is found (should be loaded into m_filePaths, currently upon initialization), load, store, then return the shader.
		if (shaderPath != m_filePaths.end())
		{
			
			std::string searchResult = m_filePaths[Name];
			//Ensure the file is a shader file, based on the extension. 
			std::string extension = searchResult.substr(searchResult.find_last_of('.') + 1);

			if (extension == "fs" || extension == "vs")
			{
				GE_CORE_INFO("[ResourceManager] " + Name + " found.");
				data = readSource(searchResult);
				
				m_shaders.insert(std::pair<std::string, std::string>(Name, data));
				
				return data;
			}
		}

		GE_CORE_INFO("[ResourceManager] " + Name + " not found.");
		return data;
	}
#pragma endregion
	
#pragma region Read Functions
	
	//Read the provided file path and return it as a string.
	std::string ResourceManager::readSource(const std::string &sourcePath)
	{
		std::string src;
		std::ifstream reader(sourcePath);
		
		//If sourcePath is found, read the file and return it as a string.
		if (reader.is_open())
		{
			std::string line;
			
			//Upon the end of each line, append a \n for a new line.
			while (std::getline(reader, line))
			{
				src.append(line.append("\n"));
			}
			reader.close();
		}
		
		return src;
	}
#pragma endregion
}