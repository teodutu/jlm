/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JLM_IR_MODULE_HPP
#define JLM_IR_MODULE_HPP

#include <jlm/ir/basic_block.hpp>
#include <jlm/ir/callgraph.hpp>
#include <jlm/ir/tac.hpp>

namespace jlm {

/* global value */

class gblvalue final : public gblvariable {
public:
	virtual
	~gblvalue();

	inline
	gblvalue(data_node * node)
	: gblvariable(node->type(), node->name(), node->linkage())
	, node_(node)
	{}

	gblvalue(const gblvalue &) = delete;

	gblvalue(gblvalue &&) = delete;

	gblvalue &
	operator=(const gblvalue &) = delete;

	gblvalue &
	operator=(gblvalue &&) = delete;

	inline data_node *
	node() const noexcept
	{
		return node_;
	}

private:
	data_node * node_;
};

static inline std::unique_ptr<jlm::gblvalue>
create_gblvalue(data_node * node)
{
	return std::make_unique<jlm::gblvalue>(node);
}

/* module */

class module final {
	typedef std::unordered_set<const jlm::gblvalue*>::const_iterator const_iterator;

public:
	inline
	~module()
	{}

	inline
	module(
		const std::string & target_triple,
		const std::string & data_layout) noexcept
	: data_layout_(data_layout)
	, target_triple_(target_triple)
	{}

	inline jlm::callgraph &
	callgraph() noexcept
	{
		return clg_;
	}

	inline const jlm::callgraph &
	callgraph() const noexcept
	{
		return clg_;
	}

	const_iterator
	begin() const
	{
		return globals_.begin();
	}

	const_iterator
	end() const
	{
		return globals_.end();
	}

	inline jlm::gblvalue *
	create_global_value(data_node * node)
	{
		auto v = jlm::create_gblvalue(node);
		auto ptr = v.get();
		globals_.insert(ptr);
		functions_[node] = ptr;
		variables_.insert(std::move(v));
		return ptr;
	}

	/*
		FIXME: tacs are supposed to be the owners of tacvariable. This is going to be removed
		       again.
	*/
	inline jlm::tacvariable *
	create_tacvariable(const jive::type & type)
	{
		static uint64_t c = 0;
		auto v = jlm::create_tacvariable(type, strfmt("tv", c++));
		auto pv = v.get();
		variables_.insert(std::move(v));
		return static_cast<tacvariable*>(pv);
	}

	inline jlm::variable *
	create_variable(const jive::type & type, const std::string & name, bool exported)
	{
		auto v = std::make_unique<jlm::variable>(type, name, exported);
		auto pv = v.get();
		variables_.insert(std::move(v));
		return pv;
	}

	inline jlm::variable *
	create_variable(const jive::type & type, bool exported)
	{
		static uint64_t c = 0;
		auto v = std::make_unique<jlm::variable>(type, strfmt("v", c++), exported);
		auto pv = v.get();
		variables_.insert(std::move(v));
		return pv;
	}

	inline jlm::variable *
	create_variable(function_node * node, const jlm::linkage & linkage)
	{
		JLM_DEBUG_ASSERT(!variable(node));

		auto v = std::unique_ptr<jlm::variable>(new fctvariable(node, linkage));
		auto pv = v.get();
		functions_[node] = pv;
		variables_.insert(std::move(v));
		return pv;
	}

	const jlm::variable *
	variable(const callgraph_node * node) const noexcept
	{
		auto it = functions_.find(node);
		return it != functions_.end() ? it->second : nullptr;
	}

	inline const std::string &
	target_triple() const noexcept
	{
		return target_triple_;
	}

	inline const std::string &
	data_layout() const noexcept
	{
		return data_layout_;
	}

private:
	jlm::callgraph clg_;
	std::string data_layout_;
	std::string target_triple_;
	std::unordered_set<const jlm::gblvalue*> globals_;
	std::unordered_set<std::unique_ptr<jlm::variable>> variables_;
	std::unordered_map<const callgraph_node*, const jlm::variable*> functions_;
};

static inline size_t
ntacs(const jlm::module & module)
{
	size_t ntacs = 0;
	for (const auto & n : module.callgraph()) {
		auto f = dynamic_cast<const function_node*>(&n);
		if (!f) continue;

		auto cfg = f->cfg();
		if (!cfg) continue;

		for (const auto & node : *f->cfg()) {
			if (auto bb = dynamic_cast<const jlm::basic_block*>(&node.attribute()))
				ntacs += bb->ntacs();
		}
	}

	return ntacs;
}

}

#endif
