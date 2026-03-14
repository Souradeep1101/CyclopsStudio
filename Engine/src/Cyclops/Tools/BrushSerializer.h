#pragma once

#include "Cyclops/Tools/Brush.h"
#include "Cyclops/Core/Base.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Cyclops
{
	class CYCLOPS_API BrushSerializer
	{
	public:
		static bool LoadBrush(const std::string& filepath, Brush& outBrush)
		{
			std::ifstream stream{ filepath };
			if (!stream.is_open())
			{
				std::cout << "[BrushSerializer] Error: Could not open file: " << filepath << std::endl;
				return false;
			}

            std::string line;
            while (std::getline(stream, line))
            {
                if (line.empty()) continue;

                std::stringstream ss(line);
                std::string key;
                ss >> key;

                if (key == "Name:")
                {
                    // Read the rest of the line
                    std::getline(ss, outBrush.Name);
                    // Trim leading space if present
                    if (!outBrush.Name.empty() && outBrush.Name[0] == ' ')
                        outBrush.Name.erase(0, 1);
                }
                else if (key == "Texture:")
                {
                    std::string path;
                    ss >> path;
                    outBrush.SetTexture(path);
                }
                else if (key == "Size:")
                {
                    ss >> outBrush.Size;
                }
                else if (key == "Spacing:")
                {
                    ss >> outBrush.Spacing;
                }
                else if (key == "Color:")
                {
                    ss >> outBrush.Color.r >> outBrush.Color.g >> outBrush.Color.b >> outBrush.Color.a;
                }
            }

            return true;
		}
	};
}
