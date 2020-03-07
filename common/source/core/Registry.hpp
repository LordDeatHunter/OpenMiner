/*
 * =====================================================================================
 *
 *  OpenMiner
 *
 *  Copyright (C) 2018-2020 Unarelith, Quentin Bazin <openminer@unarelith.net>
 *  Copyright (C) 2019-2020 the OpenMiner contributors (see CONTRIBUTORS.md)
 *
 *  This file is part of OpenMiner.
 *
 *  OpenMiner is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  OpenMiner is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with OpenMiner; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * =====================================================================================
 */
#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <memory>
#include <unordered_map>
#include <vector>

#include "Biome.hpp"
#include "Block.hpp"
#include "Item.hpp"
#include "Network.hpp"
#include "Recipe.hpp"
#include "Tree.hpp"

class Registry : public ISerializable {
	public:
		template<typename T>
		auto registerBlock(const TilesDef &tiles, const std::string &stringID, const std::string &label) -> typename std::enable_if<std::is_base_of<Block, T>::value, T&>::type {
			u32 id = m_blocks.size();
			m_blocksID.emplace(stringID, id);
			m_blocks.emplace_back(std::make_unique<T>(id, tiles, stringID, label));
			return *static_cast<T*>(m_blocks.back().get());
		}

		template<typename T>
		auto registerSerializedBlock(sf::Packet &packet) -> typename std::enable_if<std::is_base_of<Block, T>::value, T&>::type {
			m_blocks.emplace_back(std::make_unique<T>());
			m_blocks.back()->deserialize(packet);

			u32 id = m_blocks.size() - 1;
			m_blocksID.emplace(m_blocks.back()->stringID(), id);

			return *static_cast<T*>(m_blocks.back().get());
		}

		Item &registerItem(const TilesDef &tiles, const std::string &stringID, const std::string &name);
		Item &registerSerializedItem(sf::Packet &packet);

		template<typename T, typename... Args>
		auto registerRecipe(Args &&...args) -> typename std::enable_if<std::is_base_of<Recipe, T>::value, Recipe*>::type {
			m_recipes.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			return m_recipes.back().get();
		}

		Tree &registerTree(const std::string &stringID, const std::string &label);
		Tree &registerSerializedTree(sf::Packet &packet);

		Biome &registerBiome(const std::string &stringID, const std::string &label);
		Biome &registerSerializedBiome(sf::Packet &packet);

		const Block &getBlock(std::size_t id) const { return *m_blocks.at(id).get(); }
		const Item &getItem(std::size_t id) const { return m_items.at(id); }

		const Block &getBlockFromStringID(const std::string &stringID);
		const Item &getItemFromStringID(const std::string &stringID);
		const Tree &getTreeFromStringID(const std::string &stringID);
		const Biome &getBiomeFromStringID(const std::string &stringID);

		const Recipe *getRecipe(const Inventory &inventory) const;

		const Tree &getTree(u32 id) const { return m_trees.at(id); }
		const Biome &getBiome(u32 id) const { return m_biomes.at(id); }

		void serialize(sf::Packet &packet) const override;
		void deserialize(sf::Packet &packet) override;

		const std::vector<std::unique_ptr<Block>> &blocks() const { return m_blocks; }
		const std::vector<Item> &items() const { return m_items; }
		const std::vector<Tree> &trees() const { return m_trees; }
		const std::vector<Biome> &biomes() const { return m_biomes; }

		static Registry &getInstance() { return *s_instance; }
		static void setInstance(Registry &instance) { s_instance = &instance; }

	private:
		static Registry *s_instance;

		std::vector<std::unique_ptr<Block>> m_blocks;
		std::vector<Item> m_items;
		std::vector<std::unique_ptr<Recipe>> m_recipes;
		std::vector<Tree> m_trees;
		std::vector<Biome> m_biomes;

		std::unordered_map<std::string, u32> m_blocksID;
		std::unordered_map<std::string, u32> m_itemsID;
		std::unordered_map<std::string, u16> m_treesID;
		std::unordered_map<std::string, u16> m_biomesID;

		enum class DataType {
			Block,
			Item,
			CraftingRecipe,
			SmeltingRecipe,
			Tree,
			Biome
		};
};

#endif // REGISTRY_HPP_