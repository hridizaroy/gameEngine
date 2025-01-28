#pragma once

#include "config.h"
#include <filesystem>

namespace vkUtil
{
	std::vector<char> readFile(std::string filename, bool debug)
	{
		// start the stream at end of file, in order to get file size
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (debug && !file.is_open())
		{
			std::filesystem::path cwd = std::filesystem::current_path();
			std::cout << "Failed to load \"" << filename << "\"\nCWD: " << cwd << std::endl;
		}

		// Get number of bytes
		size_t filesize{ static_cast<size_t>(file.tellg()) };

		std::vector<char> buffer(filesize);

		// Go to start of file
		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();

		return buffer;
	}


	vk::ShaderModule createModule(std::string filename, vk::Device device, bool debug)
	{
		std::vector<char> sourceCode = readFile(filename, debug);
		
		vk::ShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.flags = vk::ShaderModuleCreateFlags();
		moduleInfo.codeSize = sourceCode.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

		try
		{
			return device.createShaderModule(moduleInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Failed to create shader module for \"" << filename << "\"" << std::endl;
			}
		}
	}
}
