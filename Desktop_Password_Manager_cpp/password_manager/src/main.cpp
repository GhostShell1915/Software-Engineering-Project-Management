#include "vault.hpp"
#include "crypto.hpp"

#include <iostream>
#include <string>
#include <limits>
#include <iomanip>

using namespace pm;

void clear_input() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string read_line(const std::string& prompt, bool /*hidden*/ = false) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int read_int(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int v;
        if (std::cin >> v) {
            clear_input();
            return v;
        }
        clear_input();
        std::cout << "Please enter a number.\n";
    }
}

void print_header() {
    std::cout << "\n========================================\n"
              << "   Desktop Password Manager (C++)\n"
              << "========================================\n";
}

void print_credential(const Credential& c, bool show_password = false) {
    std::cout << "  ID      : " << c.id << "\n"
              << "  Service : " << c.service << "\n"
              << "  Username: " << c.username << "\n"
              << "  Password: " << (show_password ? c.password : "********") << "\n";
    if (!c.notes.empty()) {
        std::cout << "  Notes   : " << c.notes << "\n";
    }
    std::cout << "  -----------------------------\n";
}

bool do_create_vault(Vault& vault) {
    print_header();
    std::cout << "No vault found. Let's create one.\n\n";

    std::string pw1 = read_line("Choose a master password: ", true);
    if (pw1.empty()) {
        std::cout << "Password cannot be empty.\n";
        return false;
    }
    std::string pw2 = read_line("Confirm master password : ", true);
    if (pw1 != pw2) {
        std::cout << "Passwords do not match.\n";
        secure_wipe(pw1);
        secure_wipe(pw2);
        return false;
    }

    try {
        vault.create(pw1);
        std::cout << "\nVault created successfully. You are now logged in.\n";
        secure_wipe(pw1);
        secure_wipe(pw2);
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error creating vault: " << e.what() << "\n";
        secure_wipe(pw1);
        secure_wipe(pw2);
        return false;
    }
}

bool do_login(Vault& vault) {
    print_header();
    std::cout << "Please log in.\n\n";

    const int MAX_ATTEMPTS = 5;
    for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt) {
        std::string pw = read_line("Master password: ", true);
        try {
            if (vault.unlock(pw)) {
                std::cout << "\nLogin successful.\n";
                secure_wipe(pw);
                return true;
            }
            std::cout << "Incorrect password";
            if (attempt < MAX_ATTEMPTS) {
                std::cout << " (attempt " << attempt << "/" << MAX_ATTEMPTS << ")";
            }
            std::cout << ".\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
            secure_wipe(pw);
            return false;
        }
        secure_wipe(pw);
    }
    std::cout << "Too many failed attempts. Exiting.\n";
    return false;
}

void do_list(Vault& vault, bool show_passwords = false) {
    auto entries = vault.list();
    if (entries.empty()) {
        std::cout << "\nVault is empty.\n";
        return;
    }
    std::cout << "\n--- Stored credentials (" << entries.size() << ") ---\n";
    for (const auto& c : entries) {
        print_credential(c, show_passwords);
    }
}

void do_add(Vault& vault) {
    std::cout << "\n--- Add new credential ---\n";
    std::string service  = read_line("Service / website : ");
    std::string username = read_line("Username          : ");
    std::string password = read_line("Password          : ", true);
    std::string notes    = read_line("Notes (optional)  : ");

    try {
        int id = vault.add(service, username, password, notes);
        std::cout << "Added (ID " << id << ").\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    secure_wipe(password);
}

void do_view(Vault& vault) {
    int id = read_int("Enter ID to view: ");
    auto c = vault.get(id);
    if (!c) {
        std::cout << "No entry with that ID.\n";
        return;
    }
    std::cout << "\n";
    print_credential(*c, true);
}

void do_edit(Vault& vault) {
    int id = read_int("Enter ID to edit: ");
    auto existing = vault.get(id);
    if (!existing) {
        std::cout << "No entry with that ID.\n";
        return;
    }
    std::cout << "Leave a field blank to keep the current value.\n";
    std::cout << "Current service : " << existing->service << "\n";
    std::string service = read_line("New service     : ");
    if (service.empty()) service = existing->service;

    std::cout << "Current username: " << existing->username << "\n";
    std::string username = read_line("New username    : ");
    if (username.empty()) username = existing->username;

    std::string password = read_line("New password (blank = keep): ", true);
    if (password.empty()) password = existing->password;

    std::cout << "Current notes   : " << existing->notes << "\n";
    std::string notes = read_line("New notes       : ");
    if (notes.empty()) notes = existing->notes;

    if (vault.update(id, service, username, password, notes)) {
        std::cout << "Updated.\n";
    } else {
        std::cout << "Update failed.\n";
    }
    secure_wipe(password);
}

void do_delete(Vault& vault) {
    int id = read_int("Enter ID to delete: ");
    auto c = vault.get(id);
    if (!c) {
        std::cout << "No entry with that ID.\n";
        return;
    }
    std::cout << "About to delete:\n";
    print_credential(*c, false);
    std::string confirm = read_line("Type YES to confirm: ");
    if (confirm == "YES") {
        if (vault.remove(id)) {
            std::cout << "Deleted.\n";
        } else {
            std::cout << "Delete failed.\n";
        }
    } else {
        std::cout << "Cancelled.\n";
    }
}

void do_search(Vault& vault) {
    std::string q = read_line("Search service name: ");
    auto results = vault.search(q);
    if (results.empty()) {
        std::cout << "No matches.\n";
        return;
    }
    std::cout << "\n--- Search results (" << results.size() << ") ---\n";
    for (const auto& c : results) {
        print_credential(c, false);
    }
}

void main_menu(Vault& vault) {
    while (vault.is_unlocked()) {
        std::cout << "\n========== Main Menu ==========\n"
                  << "  1. List all credentials\n"
                  << "  2. View a credential (show password)\n"
                  << "  3. Add credential\n"
                  << "  4. Edit credential\n"
                  << "  5. Delete credential\n"
                  << "  6. Search by service name\n"
                  << "  7. Logout\n"
                  << "  8. Exit\n"
                  << "================================\n";
        int choice = read_int("Choice: ");

        try {
            switch (choice) {
                case 1: do_list(vault, false); break;
                case 2: do_view(vault); break;
                case 3: do_add(vault); break;
                case 4: do_edit(vault); break;
                case 5: do_delete(vault); break;
                case 6: do_search(vault); break;
                case 7: vault.lock(); break;
                case 8: return;
                default: std::cout << "Invalid choice.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
}

int main() {
    try {
        Vault vault("vault.dat");
        if (!vault.exists()) {
            if (!do_create_vault(vault)) return 1;
        } else {
            if (!do_login(vault)) return 1;
        }
        main_menu(vault);
    } catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
