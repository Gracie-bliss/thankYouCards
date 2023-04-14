#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <functional>
#include <algorithm>

struct Node {
    int tag;
    Node* next;
    std::mutex lock;
};

class ConcurrentLinkedList {
public:
    ConcurrentLinkedList() : head_(nullptr) {}

    void add(int tag) {
        Node* new_node = new Node{ tag, nullptr };

        std::unique_lock<std::mutex> lock(head_mutex_);
        Node* curr = head_;
        Node* prev = nullptr;

        while (curr != nullptr && curr->tag < tag) {
            prev = curr;
            curr = curr->next;
        }

        if (prev == nullptr) {
            head_ = new_node;
        }
        else {
            prev->next = new_node;
        }

        new_node->next = curr;
    }

    int remove(int tag) {
        std::unique_lock<std::mutex> lock(head_mutex_);
        Node* curr = head_;
        Node* prev = nullptr;

        while (curr != nullptr && curr->tag < tag) {
            prev = curr;
            curr = curr->next;
        }

        if (curr == nullptr || curr->tag != tag) {
            return -1;
        }

        if (prev == nullptr) {
            head_ = curr->next;
        }
        else {
            prev->next = curr->next;
        }

        int removed_tag = curr->tag;
        delete curr;

        return removed_tag;
    }

    bool contains(int tag) {
        std::unique_lock<std::mutex> lock(head_mutex_);
        Node* curr = head_;

        while (curr != nullptr && curr->tag < tag) {
            curr = curr->next;
        }

        return (curr != nullptr && curr->tag == tag);
    }

private:
    Node* head_;
    std::mutex head_mutex_;
};



int generatePresent(std::mt19937& rng, int start, int end) {
    std::uniform_int_distribution<int> dist(start, end);
    return dist(rng);
}

void performOperations(ConcurrentLinkedList& list, std::mt19937& rng, int start, int end, int num, std::vector<int>& bag) {
    int index = start;
    for (int i = 0; i < num*2; i++) {
        int tag = generatePresent(rng, start, end);
        if (!list.contains(bag.at(index))) {
            list.add(bag.at(index));
        }
        if (i % 2 == 1) {
            list.remove(bag.at(index));
            index++;
        }
        bool lookup = list.contains(tag);

    }
}


int main() {
   
    std::random_device rd;
    std::mt19937 rng(rd());

   
    ConcurrentLinkedList list;

    const int N = 500000;
    std::vector<int> bag(N);

    for (int i = 0; i < N; i++) {
        bag[i] = i + 1;
    }

    std::shuffle(bag.begin(), bag.end(), rng);

    
    int num = 500000 / 4;

    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(performOperations, std::ref(list), std::ref(rng), num*(i+1)-num, num*(i+1), num, std::ref(bag));
    }

    
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}

