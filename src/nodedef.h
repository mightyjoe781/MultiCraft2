/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include "irrlichttypes_bloated.h"
#include <string>
#include <iostream>
#include <map>
#include "mapnode.h"
#include "nameidmapping.h"
#ifndef SERVER
#include "client/tile.h"
#include <IMeshManipulator.h>
class Client;
#endif
#include "itemgroup.h"
#include "sound.h" // SimpleSoundSpec
#include "constants.h" // BS
#include "texture_override.h" // TextureOverride
#include "tileanimation.h"

// PROTOCOL_VERSION >= 37
static const u8 CONTENTFEATURES_VERSION = 13;

class IItemDefManager;
class ITextureSource;
class IShaderSource;
class IGameDef;
class NodeResolver;

enum ContentParamType
{
	CPT_NONE,
	CPT_LIGHT,
};

enum ContentParamType2
{
	CPT2_NONE,
	// Need 8-bit param2
	CPT2_FULL,
	// Flowing liquid properties
	CPT2_FLOWINGLIQUID,
	// Direction for chests and furnaces and such
	CPT2_FACEDIR,
	// Direction for signs, torches and such
	CPT2_WALLMOUNTED,
	// Block level like FLOWINGLIQUID
	CPT2_LEVELED,
	// 2D rotation for things like plants
	CPT2_DEGROTATE,
	// Mesh options for plants
	CPT2_MESHOPTIONS,
	// Index for palette
	CPT2_COLOR,
	// 3 bits of palette index, then facedir
	CPT2_COLORED_FACEDIR,
	// 5 bits of palette index, then wallmounted
	CPT2_COLORED_WALLMOUNTED,
	// Glasslike framed drawtype internal liquid level, param2 values 0 to 63
	CPT2_GLASSLIKE_LIQUID_LEVEL,
};

enum LiquidType
{
	LIQUID_NONE,
	LIQUID_FLOWING,
	LIQUID_SOURCE,
};

enum NodeBoxType
{
	NODEBOX_REGULAR, // Regular block; allows buildable_to
	NODEBOX_FIXED, // Static separately defined box(es)
	NODEBOX_WALLMOUNTED, // Box for wall mounted nodes; (top, bottom, side)
	NODEBOX_LEVELED, // Same as fixed, but with dynamic height from param2. for snow, ...
	NODEBOX_CONNECTED, // optionally draws nodeboxes if a neighbor node attaches
};

struct NodeBox
{
	enum NodeBoxType type;
	// NODEBOX_REGULAR (no parameters)
	// NODEBOX_FIXED
	std::vector<aabb3f> fixed;
	// NODEBOX_WALLMOUNTED
	aabb3f wall_top;
	aabb3f wall_bottom;
	aabb3f wall_side; // being at the -X side
	// NODEBOX_CONNECTED
	std::vector<aabb3f> connect_top;
	std::vector<aabb3f> connect_bottom;
	std::vector<aabb3f> connect_front;
	std::vector<aabb3f> connect_left;
	std::vector<aabb3f> connect_back;
	std::vector<aabb3f> connect_right;
	std::vector<aabb3f> disconnected_top;
	std::vector<aabb3f> disconnected_bottom;
	std::vector<aabb3f> disconnected_front;
	std::vector<aabb3f> disconnected_left;
	std::vector<aabb3f> disconnected_back;
	std::vector<aabb3f> disconnected_right;
	std::vector<aabb3f> disconnected;
	std::vector<aabb3f> disconnected_sides;

	NodeBox()
	{ reset(); }

	void reset();
	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is);
};

struct MapNode;
class NodeMetadata;

enum LeavesStyle {
	LEAVES_FANCY,
	LEAVES_SIMPLE,
	LEAVES_OPAQUE,
};

enum AutoScale : u8 {
	AUTOSCALE_DISABLE,
	AUTOSCALE_ENABLE,
	AUTOSCALE_FORCE,
};

enum WorldAlignMode : u8 {
	WORLDALIGN_DISABLE,
	WORLDALIGN_ENABLE,
	WORLDALIGN_FORCE,
	WORLDALIGN_FORCE_NODEBOX,
};

class TextureSettings {
public:
	LeavesStyle leaves_style;
	WorldAlignMode world_aligned_mode;
	AutoScale autoscale_mode;
	int node_texture_size;
	bool opaque_water;
	bool connected_glass;
	bool use_normal_texture;
	bool enable_mesh_cache;
	bool enable_minimap;

	TextureSettings() = default;

	void readSettings();
};

enum NodeDrawType
{
	// A basic solid block
	NDT_NORMAL,
	// Nothing is drawn
	NDT_AIRLIKE,
	// Do not draw face towards same kind of flowing/source liquid
	NDT_LIQUID,
	// A very special kind of thing
	NDT_FLOWINGLIQUID,
	// Glass-like, don't draw faces towards other glass
	NDT_GLASSLIKE,
	// Leaves-like, draw all faces no matter what
	NDT_ALLFACES,
	// Enabled -> ndt_allfaces, disabled -> ndt_normal
	NDT_ALLFACES_OPTIONAL,
	// Single plane perpendicular to a surface
	NDT_TORCHLIKE,
	// Single plane parallel to a surface
	NDT_SIGNLIKE,
	// 2 vertical planes in a 'X' shape diagonal to XZ axes.
	// paramtype2 = "meshoptions" allows various forms, sizes and
	// vertical and horizontal random offsets.
	NDT_PLANTLIKE,
	// Fenceposts that connect to neighbouring fenceposts with horizontal bars
	NDT_FENCELIKE,
	// Selects appropriate junction texture to connect like rails to
	// neighbouring raillikes.
	NDT_RAILLIKE,
	// Custom Lua-definable structure of multiple cuboids
	NDT_NODEBOX,
	// Glass-like, draw connected frames and all visible faces.
	// param2 > 0 defines 64 levels of internal liquid
	// Uses 3 textures, one for frames, second for faces,
	// optional third is a 'special tile' for the liquid.
	NDT_GLASSLIKE_FRAMED,
	// Draw faces slightly rotated and only on neighbouring nodes
	NDT_FIRELIKE,
	// Enabled -> ndt_glasslike_framed, disabled -> ndt_glasslike
	NDT_GLASSLIKE_FRAMED_OPTIONAL,
	// Uses static meshes
	NDT_MESH,
	// Combined plantlike-on-solid
	NDT_PLANTLIKE_ROOTED,
};

// Mesh options for NDT_PLANTLIKE with CPT2_MESHOPTIONS
static const u8 MO_MASK_STYLE          = 0x07;
static const u8 MO_BIT_RANDOM_OFFSET   = 0x08;
static const u8 MO_BIT_SCALE_SQRT2     = 0x10;
static const u8 MO_BIT_RANDOM_OFFSET_Y = 0x20;
enum PlantlikeStyle {
	PLANT_STYLE_CROSS,
	PLANT_STYLE_CROSS2,
	PLANT_STYLE_STAR,
	PLANT_STYLE_HASH,
	PLANT_STYLE_HASH2,
};

enum AlignStyle : u8 {
	ALIGN_STYLE_NODE,
	ALIGN_STYLE_WORLD,
	ALIGN_STYLE_USER_DEFINED,
};

/*
	Stand-alone definition of a TileSpec (basically a server-side TileSpec)
*/

struct TileDef
{
	std::string name = "";
	bool backface_culling = true; // Takes effect only in special cases
	bool tileable_horizontal = true;
	bool tileable_vertical = true;
	//! If true, the tile has its own color.
	bool has_color = false;
	//! The color of the tile.
	video::SColor color = video::SColor(0xFFFFFFFF);
	AlignStyle align_style = ALIGN_STYLE_NODE;
	u8 scale = 0;

	struct TileAnimationParams animation;

	TileDef()
	{
		animation.type = TAT_NONE;
	}

	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is, u8 contentfeatures_version,
		NodeDrawType drawtype);
};

// Defines the number of special tiles per nodedef
//
// NOTE: When changing this value, the enum entries of OverrideTarget and
//       parser in TextureOverrideSource must be updated so that all special
//       tiles can be overridden.
#define CF_SPECIAL_COUNT 6

struct ContentFeatures
{
	/*
		Cached stuff
	 */
#ifndef SERVER
	// 0     1     2     3     4     5
	// up    down  right left  back  front
	TileSpec tiles[6];
	// Special tiles
	// - Currently used for flowing liquids
	TileSpec special_tiles[CF_SPECIAL_COUNT];
	u8 solidness; // Used when choosing which face is drawn
	u8 visual_solidness; // When solidness=0, this tells how it looks like
	bool backface_culling;
#endif

	// Server-side cached callback existence for fast skipping
	bool has_on_construct;
	bool has_on_destruct;
	bool has_after_destruct;

	/*
		Actual data
	 */

	// --- GENERAL PROPERTIES ---

	std::string name; // "" = undefined node
	ItemGroupList groups; // Same as in itemdef
	// Type of MapNode::param1
	ContentParamType param_type;
	// Type of MapNode::param2
	ContentParamType2 param_type_2;

	// --- VISUAL PROPERTIES ---

	enum NodeDrawType drawtype;
	std::string mesh;
#ifndef SERVER
	scene::IMesh *mesh_ptr[24];
	video::SColor minimap_color;
#endif
	float visual_scale; // Misc. scale parameter
	TileDef tiledef[6];
	// These will be drawn over the base tiles.
	TileDef tiledef_overlay[6];
	TileDef tiledef_special[CF_SPECIAL_COUNT]; // eg. flowing liquid
	// If 255, the node is opaque.
	// Otherwise it uses texture alpha.
	u8 alpha;
	// The color of the node.
	video::SColor color;
	std::string palette_name;
	std::vector<video::SColor> *palette;
	// Used for waving leaves/plants
	u8 waving;
	// for NDT_CONNECTED pairing
	u8 connect_sides;
	std::vector<std::string> connects_to;
	std::vector<content_t> connects_to_ids;
	// Post effect color, drawn when the camera is inside the node.
	video::SColor post_effect_color;
	// Flowing liquid or leveled nodebox, value = default level
	u8 leveled;
	// Maximum value for leveled nodes
	u8 leveled_max;

	// --- LIGHTING-RELATED ---

	bool light_propagates;
	bool sunlight_propagates;
	// Amount of light the node emits
	u8 light_source;

	// --- MAP GENERATION ---

	// True for all ground-like things like stone and mud, false for eg. trees
	bool is_ground_content;

	// --- INTERACTION PROPERTIES ---

	// This is used for collision detection.
	// Also for general solidness queries.
	bool walkable;
	// Player can point to these
	bool pointable;
	// Player can dig these
	bool diggable;
	// Player can climb these
	bool climbable;
	// Player can build on these
	bool buildable_to;
	// Player cannot build to these (placement prediction disabled)
	bool rightclickable;
	u32 damage_per_second;
	// client dig prediction
	std::string node_dig_prediction;

	// --- LIQUID PROPERTIES ---

	// Whether the node is non-liquid, source liquid or flowing liquid
	enum LiquidType liquid_type;
	// If the content is liquid, this is the flowing version of the liquid.
	std::string liquid_alternative_flowing;
	content_t liquid_alternative_flowing_id;
	// If the content is liquid, this is the source version of the liquid.
	std::string liquid_alternative_source;
	content_t liquid_alternative_source_id;
	// Viscosity for fluid flow, ranging from 1 to 7, with
	// 1 giving almost instantaneous propagation and 7 being
	// the slowest possible
	u8 liquid_viscosity;
	// Is liquid renewable (new liquid source will be created between 2 existing)
	bool liquid_renewable;
	// Number of flowing liquids surrounding source
	u8 liquid_range;
	u8 drowning;
	// Liquids flow into and replace node
	bool floodable;

	// --- NODEBOXES ---

	NodeBox node_box;
	NodeBox selection_box;
	NodeBox collision_box;

	// --- SOUND PROPERTIES ---

	SimpleSoundSpec sound_footstep;
	SimpleSoundSpec sound_dig;
	SimpleSoundSpec sound_dug;

	// --- LEGACY ---

	// Compatibility with old maps
	// Set to true if paramtype used to be 'facedir_simple'
	bool legacy_facedir_simple;
	// Set to true if wall_mounted used to be set to true
	bool legacy_wallmounted;

	/*
		Methods
	*/

	ContentFeatures();
	~ContentFeatures();
	void reset();
	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is);
	void deSerializeOld(std::istream &is, int version);
	void serializeOld(std::ostream &os, u16 protocol_version) const;
	/*!
	 * Since vertex alpha is no longer supported, this method
	 * adds opacity directly to the texture pixels.
	 *
	 * \param tiles array of the tile definitions.
	 * \param length length of tiles
	 */
	void correctAlpha(TileDef *tiles, int length);

	/*
		Some handy methods
	*/
	bool needsBackfaceCulling() const
	{
		switch (drawtype) {
		case NDT_TORCHLIKE:
		case NDT_SIGNLIKE:
		case NDT_FIRELIKE:
		case NDT_RAILLIKE:
		case NDT_PLANTLIKE:
		case NDT_PLANTLIKE_ROOTED:
		case NDT_MESH:
			return false;
		default:
			return true;
		}
	}

	bool isLiquid() const{
		return (liquid_type != LIQUID_NONE);
	}
	bool sameLiquid(const ContentFeatures &f) const{
		if(!isLiquid() || !f.isLiquid()) return false;
		return (liquid_alternative_flowing_id == f.liquid_alternative_flowing_id);
	}

	int getGroup(const std::string &group) const
	{
		return itemgroup_get(groups, group);
	}

#ifndef SERVER
	void updateTextures(ITextureSource *tsrc, IShaderSource *shdsrc,
		scene::IMeshManipulator *meshmanip, Client *client, const TextureSettings &tsettings);
#endif
};

/*!
 * @brief This class is for getting the actual properties of nodes from their
 * content ID.
 *
 * @details The nodes on the map are represented by three numbers (see MapNode).
 * The first number (param0) is the type of a node. All node types have own
 * properties (see ContentFeatures). This class is for storing and getting the
 * properties of nodes.
 * The manager is first filled with registered nodes, then as the game begins,
 * functions only get `const` pointers to it, to prevent modification of
 * registered nodes.
 */
class NodeDefManager {
public:
	/*!
	 * Creates a NodeDefManager, and registers three ContentFeatures:
	 * \ref CONTENT_AIR, \ref CONTENT_UNKNOWN and \ref CONTENT_IGNORE.
	 */
	NodeDefManager();
	~NodeDefManager();

	/*!
	 * Returns the properties for the given content type.
	 * @param c content type of a node
	 * @return properties of the given content type, or \ref CONTENT_UNKNOWN
	 * if the given content type is not registered.
	 */
	inline const ContentFeatures& get(content_t c) const {
		return
			c < m_content_features.size() ?
				m_content_features[c] : m_content_features[CONTENT_UNKNOWN];
	}

	/*!
	 * Returns the properties of the given node.
	 * @param n a map node
	 * @return properties of the given node or @ref CONTENT_UNKNOWN if the
	 * given content type is not registered.
	 */
	inline const ContentFeatures& get(const MapNode &n) const {
		return get(n.getContent());
	}

	/*!
	 * Returns the node properties for a node name.
	 * @param name name of a node
	 * @return properties of the given node or @ref CONTENT_UNKNOWN if
	 * not found
	 */
	const ContentFeatures& get(const std::string &name) const;

	/*!
	 * Returns the content ID for the given name.
	 * @param name a node name
	 * @param[out] result will contain the content ID if found, otherwise
	 * remains unchanged
	 * @return true if the ID was found, false otherwise
	 */
	bool getId(const std::string &name, content_t &result) const;

	/*!
	 * Returns the content ID for the given name.
	 * @param name a node name
	 * @return ID of the node or @ref CONTENT_IGNORE if not found
	 */
	content_t getId(const std::string &name) const;

	/*!
	 * Returns the content IDs of the given node name or node group name.
	 * Group names start with "group:".
	 * @param name a node name or node group name
	 * @param[out] result will be appended with matching IDs
	 * @return true if `name` is a valid node name or a (not necessarily
	 * valid) group name
	 */
	bool getIds(const std::string &name, std::vector<content_t> &result) const;

	/*!
	 * Returns the smallest box in integer node coordinates that
	 * contains all nodes' selection boxes. The returned box might be larger
	 * than the minimal size if the largest node is removed from the manager.
	 */
	inline core::aabbox3d<s16> getSelectionBoxIntUnion() const {
		return m_selection_box_int_union;
	}

	/*!
	 * Checks whether a node connects to an adjacent node.
	 * @param from the node to be checked
	 * @param to the adjacent node
	 * @param connect_face a bit field indicating which face of the node is
	 * adjacent to the other node.
	 * Bits: +y (least significant), -y, -z, -x, +z, +x (most significant).
	 * @return true if the node connects, false otherwise
	 */
	bool nodeboxConnects(MapNode from, MapNode to,
			u8 connect_face) const;

	/*!
	 * Registers a NodeResolver to wait for the registration of
	 * ContentFeatures. Once the node registration finishes, all
	 * listeners are notified.
	 */
	void pendNodeResolve(NodeResolver *nr) const;

	/*!
	 * Stops listening to the NodeDefManager.
	 * @return true if the listener was registered before, false otherwise
	 */
	bool cancelNodeResolveCallback(NodeResolver *nr) const;

	/*!
	 * Registers a new node type with the given name and allocates a new
	 * content ID.
	 * Should not be called with an already existing name.
	 * @param name name of the node, must match with `def.name`.
	 * @param def definition of the registered node type.
	 * @return ID of the registered node or @ref CONTENT_IGNORE if
	 * the function could not allocate an ID.
	 */
	content_t set(const std::string &name, const ContentFeatures &def);

	/*!
	 * Allocates a blank node ID for the given name.
	 * @param name name of a node
	 * @return allocated ID or @ref CONTENT_IGNORE if could not allocate
	 * an ID.
	 */
	content_t allocateDummy(const std::string &name);

	/*!
	 * Removes the given node name from the manager.
	 * The node ID will remain in the manager, but won't be linked to any name.
	 * @param name name to be removed
	 */
	void removeNode(const std::string &name);

	/*!
	 * Regenerates the alias list (a map from names to node IDs).
	 * @param idef the item definition manager containing alias information
	 */
	void updateAliases(IItemDefManager *idef);

	/*!
	 * Replaces the textures of registered nodes with the ones specified in
	 * the texturepack's override.txt file
	 *
	 * @param overrides the texture overrides
	 */
	void applyTextureOverrides(const std::vector<TextureOverride> &overrides);

	/*!
	 * Only the client uses this. Loads textures and shaders required for
	 * rendering the nodes.
	 * @param gamedef must be a Client.
	 * @param progress_cbk called each time a node is loaded. Arguments:
	 * `progress_cbk_args`, number of loaded ContentFeatures, number of
	 * total ContentFeatures.
	 * @param progress_cbk_args passed to the callback function
	 */
	void updateTextures(IGameDef *gamedef,
		void (*progress_cbk)(void *progress_args, u32 progress, u32 max_progress),
		void *progress_cbk_args);

	/*!
	 * Writes the content of this manager to the given output stream.
	 * @param protocol_version serialization version of ContentFeatures
	 */
	void serialize(std::ostream &os, u16 protocol_version) const;

	/*!
	 * Restores the manager from a serialized stream.
	 * This clears the previous state.
	 * @param is input stream containing a serialized NodeDefManager
	 */
	void deSerialize(std::istream &is);

	/*!
	 * Used to indicate that node registration has finished.
	 * @param completed tells whether registration is complete
	 */
	inline void setNodeRegistrationStatus(bool completed) {
		m_node_registration_complete = completed;
	}

	/*!
	 * Notifies the registered NodeResolver instances that node registration
	 * has finished, then unregisters all listeners.
	 * Must be called after node registration has finished!
	 */
	void runNodeResolveCallbacks();

	/*!
	 * Sets the registration completion flag to false and unregisters all
	 * NodeResolver instances listening to the manager.
	 */
	void resetNodeResolveState();

	/*!
	 * Resolves (caches the IDs) cross-references between nodes,
	 * like liquid alternatives.
	 * Must be called after node registration has finished!
	 */
	void resolveCrossrefs();

private:
	/*!
	 * Resets the manager to its initial state.
	 * See the documentation of the constructor.
	 */
	void clear();

	/*!
	 * Allocates a new content ID, and returns it.
	 * @return the allocated ID or \ref CONTENT_IGNORE if could not allocate
	 */
	content_t allocateId();

	/*!
	 * Binds the given content ID and node name.
	 * Registers them in \ref m_name_id_mapping and
	 * \ref m_name_id_mapping_with_aliases.
	 * @param i a content ID
	 * @param name a node name
	 */
	void addNameIdMapping(content_t i, std::string name);

	/*!
	 * Removes a content ID from all groups.
	 * Erases content IDs from vectors in \ref m_group_to_items and
	 * removes empty vectors.
	 * @param id Content ID
	 */
	void eraseIdFromGroups(content_t id);

	/*!
	 * Recalculates m_selection_box_int_union based on
	 * m_selection_box_union.
	 */
	void fixSelectionBoxIntUnion();

	//! Features indexed by ID.
	std::vector<ContentFeatures> m_content_features;

	//! A mapping for fast conversion between names and IDs
	NameIdMapping m_name_id_mapping;

	/*!
	 * Like @ref m_name_id_mapping, but maps only from names to IDs, and
	 * includes aliases too. Updated by \ref updateAliases().
	 * Note: Not serialized.
	 */
	std::unordered_map<std::string, content_t> m_name_id_mapping_with_aliases;

	/*!
	 * A mapping from group names to a vector of content types that belong
	 * to it. Necessary for a direct lookup in \ref getIds().
	 * Note: Not serialized.
	 */
	std::unordered_map<std::string, std::vector<content_t>> m_group_to_items;

	/*!
	 * The next ID that might be free to allocate.
	 * It can be allocated already, because \ref CONTENT_AIR,
	 * \ref CONTENT_UNKNOWN and \ref CONTENT_IGNORE are registered when the
	 * manager is initialized, and new IDs are allocated from 0.
	 */
	content_t m_next_id;

	//! True if all nodes have been registered.
	bool m_node_registration_complete;

	/*!
	 * The union of all nodes' selection boxes.
	 * Might be larger if big nodes are removed from the manager.
	 */
	aabb3f m_selection_box_union;

	/*!
	 * The smallest box in integer node coordinates that
	 * contains all nodes' selection boxes.
	 * Might be larger if big nodes are removed from the manager.
	 */
	core::aabbox3d<s16> m_selection_box_int_union;

	/*!
	 * NodeResolver instances to notify once node registration has finished.
	 * Even constant NodeDefManager instances can register listeners.
	 */
	mutable std::vector<NodeResolver *> m_pending_resolve_callbacks;
};

NodeDefManager *createNodeDefManager();

class NodeResolver {
public:
	NodeResolver();
	virtual ~NodeResolver();
	virtual void resolveNodeNames() = 0;

	// required because this class is used as mixin for ObjDef
	void cloneTo(NodeResolver *res) const;

	bool getIdFromNrBacklog(content_t *result_out,
		const std::string &node_alt, content_t c_fallback,
		bool error_on_fallback = true);
	bool getIdsFromNrBacklog(std::vector<content_t> *result_out,
		bool all_required = false, content_t c_fallback = CONTENT_IGNORE);

	void nodeResolveInternal();

	u32 m_nodenames_idx = 0;
	u32 m_nnlistsizes_idx = 0;
	std::vector<std::string> m_nodenames;
	std::vector<size_t> m_nnlistsizes;
	const NodeDefManager *m_ndef = nullptr;
	bool m_resolve_done = false;
};
