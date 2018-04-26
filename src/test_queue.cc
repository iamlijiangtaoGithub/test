#include <memory>
#include <iostream>
#include <fstream>
#include <queue>

using namespace std;

struct city {
    int id;
    float score;
    city(int id , float score): id(id), score(score) {}
};

struct Compare {
    bool operator() (std::shared_ptr<city> l, std::shared_ptr<city> r )
    {
        return l->score < r->score;
    }
};

typedef priority_queue< std::shared_ptr<city>, vector<std::shared_ptr<city>>, Compare > pqueue;

int test_queue()
{
    pqueue cities;
    auto display = [](pqueue& cities)-> void {
        int i = 0;
        while (!cities.empty()) {
            std::shared_ptr<city> top = cities.top();
            std::cout << "the " << i++ << "th, id:" <<  top->id << "\t score:" << top->score << std::endl;
            cities.pop();
        }
    };

    std::shared_ptr<city> city_1(new city(1, 0.1));
    cities.push(city_1);

    std::shared_ptr<city> city_2(new city(2, 0.2));
    cities.push(city_2);

    std::shared_ptr<city> city_3(new city(4, 0.4));
    cities.push(city_3);
    city_1->score = 0.5;

    std::cout << "max heap, top:" << cities.top()->score << "\n";
    std::make_heap(const_cast<std::shared_ptr<city>*>(&cities.top()),
                   const_cast<std::shared_ptr<city>*>(&cities.top()) + cities.size(),
                   Compare());
    std::cout << "max heap after resort, top::" << cities.top()->score << std::endl;
    display(cities);
    return 0;
}

int main() {
    test_queue();
}

