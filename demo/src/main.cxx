#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#define algorithm_first_fit  0
#define algorithm_worst_fit  1
#define algorithm_best_fit   2

struct gte {
    explicit gte(int rhs) : m_rhs(rhs) {}
    bool operator()(int lhs) { return lhs >= m_rhs; }

    int m_rhs;
};

std::ptrdiff_t find_first_fit(int request_size, std::vector<int>& memory_partitions) {
    auto it = std::ranges::find_if(memory_partitions, gte(request_size));
    if (it != memory_partitions.end()) {
        *it -= request_size;
        return std::ranges::distance(&*memory_partitions.begin(), &*it);
    }
    return -1;
}

std::ptrdiff_t find_worst_fit(int request_size, std::vector<int>& memory_partitions) {
    auto gte_vals = memory_partitions | std::views::filter(gte(request_size));
    auto it = std::ranges::max_element(gte_vals);
    if (it != gte_vals.end()) {
        *it -= request_size;
        return std::ranges::distance(&*memory_partitions.begin(), &*it);
    }
    return -1;
}

std::ptrdiff_t find_best_fit(int request_size, std::vector<int>& memory_partitions) {
    auto gte_vals = memory_partitions | std::views::filter(gte(request_size));
    auto it = std::ranges::min_element(gte_vals);
    if (it != gte_vals.end()) {
        *it -= request_size;
        return std::ranges::distance(&*memory_partitions.begin(), &*it);
    }
    return -1;
}

int main() {
    using namespace ftxui;

    std::string inp_memory_partitions;
    Component input_memory_partitions = Input(&inp_memory_partitions, "300, 600, 350, 200, 750, 125");
    input_memory_partitions |= CatchEvent([&](Event event) {
        return event.is_character()
            && !std::isdigit(event.character()[0])  // allow numbers
            && !std::isspace(event.character()[0])  // allow spaces
            && !(event.character()[0] == ',')       // allow comma ','
        ;
    });

    std::string inp_request_sizes;
    Component input_request_sizes = Input(&inp_request_sizes, "115, 500, 358, 200, 375");
    input_request_sizes |= CatchEvent([&](Event event) {
        return event.is_character()
            && !std::isdigit(event.character()[0])  // allow numbers
            && !std::isspace(event.character()[0])  // allow spaces
            && !(event.character()[0] == ',')       // allow comma ','
        ;
    });

    std::vector<std::string> inp_algorithm_list{
        "First fit",
        "Worst fit",
        "Best fit",
    };
    int selected_algorithm = algorithm_first_fit;
    Component input_algorithm_list = Radiobox(&inp_algorithm_list, &selected_algorithm);

    auto components = Container::Vertical({
        input_memory_partitions,
        input_request_sizes,
        input_algorithm_list,
    });

    auto renderer = Renderer(components, [&]() {
        Elements elements({
            hbox({ text(" Memory partitions "), input_memory_partitions->Render() }),
            hbox({ text(" Request sizes     "), input_request_sizes->Render()     }),
            hbox({ text(" Algorithm         "), input_algorithm_list->Render()    }),
            separator(),
        });

        std::vector<int> memory_partitions;
        {
            std::stringstream ss(inp_memory_partitions);
            int v;
            while (ss >> v) {
                memory_partitions.push_back(v);
                while (ss.peek() == ',') ss.ignore();
            }
        }

        std::vector<int> request_sizes;
        {
            std::stringstream ss(inp_request_sizes);
            int v;
            while (ss >> v) {
                request_sizes.push_back(v);
                while (ss.peek() == ',') ss.ignore();
            }
        }

        for (auto&& request_size : request_sizes) {
            Elements row_elements({
                text(std::format(" {:<8d}", request_size)),
            });

            std::ptrdiff_t index;
            switch (selected_algorithm) {
            case algorithm_worst_fit:
                index = find_worst_fit(request_size, memory_partitions);
                break;

            case algorithm_best_fit:
                index = find_best_fit(request_size, memory_partitions);
                break;

            case algorithm_first_fit:
            default:
                index = find_first_fit(request_size, memory_partitions);
                break;
            }

            if (index == -1) {
                row_elements.push_back(text(" Not available"));
            }
            else {
                row_elements.push_back(text(std::format(" {:<13d}", memory_partitions[index] + request_size)));
            }

            for (auto&& [i, mem_partition] : memory_partitions | std::views::enumerate) {
                if (i == index) {
                    row_elements.push_back(text(
                        std::format(" {:<8d}", mem_partition)) | bold | bgcolor(Color::GreenLight)
                    );
                }
                else {
                    row_elements.push_back(text(std::format(" {:<8d}", mem_partition)));
                }

                if (i != (std::ptrdiff_t)memory_partitions.size()) {
                    row_elements.push_back(text(","));
                }
            }

            elements.push_back(hbox(row_elements));
        }

        return vbox(elements) | border;
    });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(renderer);
}
