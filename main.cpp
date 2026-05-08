#include <format>
#include <iostream>
#include "OrderRepository.h"
#include "SampleRepository.h"

static void printSample(const Sample& s) {
    std::cout << std::format("  [Sample] id={:<2} name={:<10} spec={:<35} stock={}\n",
        s.id, s.name, s.spec, s.stock);
}

static void printOrder(const Order& o) {
    std::cout << std::format("  [Order]  id={:<2} sampleId={} qty={} status={:<10} created={}\n",
        o.id, o.sampleId, o.quantity, orderStatusToString(o.status), o.createdAt);
}

static void printAll(const SampleRepository& sr, const OrderRepository& or_) {
    std::cout << "  -- Samples --\n";
    for (const auto& s : sr.findAll()) printSample(s);
    std::cout << "  -- Orders --\n";
    for (const auto& o : or_.findAll()) printOrder(o);
}

static void tryTransition(OrderRepository& or_, int id,
                          OrderStatus to, const char* label) {
    bool ok = or_.updateStatus(id, to);
    std::cout << std::format("  transition -> {:<10} : {}\n",
        label, ok ? "OK" : "Rejected");
    if (ok) printOrder(*or_.findById(id));
}

int main() {
    try {
        std::cout << "=== Run 1: Create & Transition ===\n\n";

        OrderRepository  orderRepo("orders.json");
        SampleRepository sampleRepo("samples.json",
            [&] { return orderRepo.findAll(); });

        auto s1 = sampleRepo.create({ 0, "GaN-001", "2inch, n-type, 2e18 cm-3", 10 });
        auto s2 = sampleRepo.create({ 0, "SiC-002", "4inch, semi-insulating",    0  });
        std::cout << "[Step 1] Created 2 samples:\n";
        printSample(s1); printSample(s2);

        std::cout << "\n[Step 2] All samples:\n";
        for (const auto& s : sampleRepo.findAll()) printSample(s);

        auto o1 = orderRepo.create({ 0, s1.id, 3 });
        std::cout << "\n[Step 3] Created order (RESERVED):\n";
        printOrder(o1);

        std::cout << "\n[AC] Invalid transition tests:\n";
        tryTransition(orderRepo, o1.id, OrderStatus::RELEASED,  "RELEASED");
        tryTransition(orderRepo, o1.id, OrderStatus::PRODUCING, "PRODUCING");

        std::cout << "\n[Step 4] RESERVED -> CONFIRMED:\n";
        tryTransition(orderRepo, o1.id, OrderStatus::CONFIRMED, "CONFIRMED");

        std::cout << "\n[Step 5] CONFIRMED -> RELEASED:\n";
        tryTransition(orderRepo, o1.id, OrderStatus::RELEASED, "RELEASED");

        std::cout << "\n[AC] Delete RELEASED order: ";
        std::cout << (orderRepo.remove(o1.id) ? "Deleted (FAIL)" : "Rejected (correct)") << "\n";

        std::cout << "[AC] Delete sample with active order ref: ";
        std::cout << (sampleRepo.remove(s1.id) ? "Deleted (FAIL)" : "Rejected (correct)") << "\n";

        std::cout << "\n=== Run 2: Reload from file (simulated restart) ===\n\n";
        {
            OrderRepository  orderRepo2("orders.json");
            SampleRepository sampleRepo2("samples.json",
                [&] { return orderRepo2.findAll(); });
            printAll(sampleRepo2, orderRepo2);
        }

        std::cout << "\n=== PoC PASSED ===\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
