#ifndef MCTS_H
#define MCTS_H

#include <omp.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <vector>

// 節點定義
template <typename State, typename Action>
class MCTSNode {
	public:
		State state;                                     // 當前遊戲狀態
		std::vector<Action> available_actions;           // 可用的動作
		std::vector<std::unique_ptr<MCTSNode>> children; // 子節點
		double wins = 0;                                 // 獲勝次數
		int visits = 0;                                  // 被訪問次數
		MCTSNode* parent = nullptr;                      // 父節點

		MCTSNode(const State& state, const std::vector<Action>& actions,
		         MCTSNode* parent = nullptr)
		    : state(state), available_actions(actions), parent(parent) {}

		// 判斷是否為葉子節點
		bool isLeaf() const { 
			bool result;
			#pragma omp critical
			{
				result = children.empty();
			}
			return result; 
		}

		// UCT公式
		double UCT(double exploration_param = 1.41) const {
			int local_visits;
			#pragma omp atomic read
			local_visits = visits; // 讀取 visits 使用 atomic 保護

			if (local_visits == 0) return std::numeric_limits<double>::infinity();

			double local_wins;
			#pragma omp atomic read
			local_wins = wins; // 讀取 wins 使用 atomic 保護

			if (parent != nullptr) {
        		int parent_visits;
        		#pragma omp atomic read
        		parent_visits = parent->visits;  // 使用 atomic 保護讀取 parent 的 visits
        		return (local_wins / local_visits) + exploration_param * std::sqrt(std::log(parent_visits) / local_visits);
    		} else {
        		// 根節點的情況下，不考慮 parent->visits
        		return local_wins / local_visits;
    		}
		}

		// 隨機選擇未展開的動作
		Action getRandomUntriedAction(std::mt19937& rng) {
			std::uniform_int_distribution<> dist(0,
			                                     available_actions.size() - 1);
			return available_actions[dist(rng)];
		}
};

// MCTS
template <typename State, typename Action>
class MCTS {
	public:
		MCTSNode<State, Action>* root; // 根節點
		double exploration_param = 1.41;
		int simulation_count = 40000;

		MCTS(const State& initial_state, const std::vector<Action>& actions) {
			root = new MCTSNode<State, Action>(initial_state, actions);
		}

		~MCTS() { deleteTree(root); }

		// 執行 MCTS
		Action run(std::mt19937& rng) {
			omp_set_num_threads(4);

			#pragma omp parallel for
			for (int i = 0; i < simulation_count; ++i) {
				// 讓每個線程都使用一個新的隨機數生成器，避免 race condition
            	std::mt19937 thread_rng(rng());

				MCTSNode<State, Action>* node = select(); // 選擇節點
				MCTSNode<State, Action>* expanded_node =
				    expand(node, rng);                        // 擴展
				double result = simulate(expanded_node, rng); // 模擬
				backpropagate(expanded_node, result);         // 回傳結果
			}
			// 返回擁有最多訪問次數的動作
			return bestAction();
		}

	private:
		// 選擇節點 (Selection)
		MCTSNode<State, Action>* select() {
			MCTSNode<State, Action>* node = root;
			while (!node->isLeaf()) {
				node = bestUCT(node);
			}
			return node;
		}

		// 使用UCT公式選擇最佳子節點
		MCTSNode<State, Action>* bestUCT(MCTSNode<State, Action>* node) {
			MCTSNode<State, Action>* best_child = nullptr;
			double best_uct = -std::numeric_limits<double>::infinity();

			#pragma omp critical
			{
				for (const auto& child : node->children) {
					double uct_value = child->UCT(exploration_param);
					if (uct_value > best_uct) {
						best_uct = uct_value;
						best_child = child.get();
					}
				}
			}

			return best_child;
		}

		// 擴展節點 (Expansion)
		MCTSNode<State, Action>* expand(MCTSNode<State, Action>* node,
		                                std::mt19937& rng) {
			if (!node->available_actions.empty()) {
				Action action = node->getRandomUntriedAction(rng);
				State next_state = node->state.applyAction(action);
				std::vector<Action> next_actions =
				    next_state.getAvailableActions();

				#pragma omp critical
				{
					node->children.push_back(std::make_unique<MCTSNode<State, Action>>(next_state, next_actions, node));
				}
				return node->children.back().get();
			}
			return node;
		}

		// 模擬 (Simulation)
		double simulate(MCTSNode<State, Action>* node, std::mt19937& rng) {
			State state = node->state;
			while (!state.isTerminal()) {
				std::vector<Action> actions = state.getAvailableActions();
				Action action = actions[std::uniform_int_distribution<>(
				    0, actions.size() - 1)(rng)];
				state = state.applyAction(action);
			}
			return state.getResult();
		}

		// 回傳 (Backpropagation)
		void backpropagate(MCTSNode<State, Action>* node, double result) {
			while (node != nullptr) {
				#pragma omp atomic  // 確保訪問次數和勝利次數的同步
				node->visits++;
				#pragma omp atomic
				node->wins += result;
				node = node->parent;
			}
		}

		// 找出最佳動作
		Action bestAction() const {
			MCTSNode<State, Action>* best_child = nullptr;
			int best_visits = -1;

			for (const auto& child : root->children) {
				if (child->visits > best_visits) {
					best_visits = child->visits;
					best_child = child.get();
				}
			}

			return best_child->state.getLastAction();
		}

		// 刪除樹節點
		void deleteTree(MCTSNode<State, Action>* node) {
			for (auto& child : node->children) {
				deleteTree(child.get());
			}
			// delete node;
		}
};

#endif