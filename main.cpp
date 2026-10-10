#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <stdexcept>
using namespace std;

// Abstract base class
class Record {
public:
    virtual void display() const = 0;
    virtual ~Record() {}
};

// Expense class
class Expense : public Record {
private:
    int id;
    string title, category, date;
    double amount;

public:
    Expense(int i, string t, string c,
            double a, string d)
        : id(i), title(t), category(c),
          amount(a), date(d) {
        if (amount <= 0)
            throw invalid_argument("Amount must be positive.");
    }

    int getId() const { return id; }
    double getAmount() const { return amount; }
    string getCategory() const { return category; }

    void display() const override {
        cout << left << setw(6) << id
             << setw(20) << title
             << setw(16) << category
             << setw(12) << fixed << setprecision(2)
             << amount << date << '\n';
    }

    void save(ofstream& file) const {
        file << id << '\n' << title << '\n'
             << category << '\n' << amount << '\n'
             << date << '\n';
    }
};

// Budget class
class Budget {
private:
    double limit;

public:
    Budget() : limit(0) {}

    void setLimit(double amount) {
        if (amount <= 0)
            throw invalid_argument("Budget must be positive.");
        limit = amount;
    }

    double getLimit() const { return limit; }

    void check(double spent) const {
        if (limit <= 0) {
            cout << "Set a budget first.\n";
        } else if (spent > limit) {
            cout << "WARNING: Budget exceeded by Rs. "
                 << fixed << setprecision(2)
                 << spent - limit << '\n';
        } else {
            cout << "Remaining budget: Rs. "
                 << fixed << setprecision(2)
                 << limit - spent << '\n';
        }
    }

    void save() const {
        ofstream file("budget.txt");
        file << limit;
    }

    void load() {
        ifstream file("budget.txt");
        if (file) file >> limit;
    }
};

// Expense manager
class ExpenseManager {
private:
    vector<Expense> expenses;
    Budget budget;
    int nextId = 1;

    void loadExpenses() {
        ifstream file("expenses.txt");
        if (!file) return;

        int id;
        string title, category, date;
        double amount;

        while (file >> id) {
            file.ignore(numeric_limits<streamsize>::max(), '\n');

            if (!getline(file, title) ||
                !getline(file, category) ||
                !(file >> amount))
                break;

            file.ignore(numeric_limits<streamsize>::max(), '\n');

            if (!getline(file, date))
                break;

            try {
                expenses.emplace_back(
                    id, title, category, amount, date);
                if (id >= nextId) nextId = id + 1;
            } catch (...) {
                // Ignore invalid saved records
            }
        }
    }

    void saveExpenses() const {
        ofstream file("expenses.txt");
        for (const auto& e : expenses)
            e.save(file);
    }

public:
    ExpenseManager() {
        loadExpenses();
        budget.load();
    }

    double total() const {
        double sum = 0;
        for (const auto& e : expenses)
            sum += e.getAmount();
        return sum;
    }

    void setBudget() {
        double amount;
        cout << "Enter budget amount (Rs.): ";
        if (!(cin >> amount) || amount <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid budget.\n";
            return;
        }

        budget.setLimit(amount);
        budget.save();
        cout << "Budget saved successfully.\n";
        budget.check(total());
    }

    void addExpense() {
        string title, category, date;
        double amount;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Expense description: ";
        getline(cin, title);

        cout << "Category (Food/Travel/Shopping/"
                "Education/Entertainment/Other): ";
        getline(cin, category);

        cout << "Amount (Rs.): ";
        if (!(cin >> amount) || amount <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid amount. Expense not added.\n";
            return;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Date (DD-MM-YYYY): ";
        getline(cin, date);

        try {
            expenses.emplace_back(
                nextId++, title, category, amount, date);
            saveExpenses();

            cout << "Expense added successfully!\n";
            budget.check(total());
        } catch (const exception& e) {
            cout << "Error: " << e.what() << '\n';
        }
    }

    void viewExpenses() const {
        if (expenses.empty()) {
            cout << "No expenses recorded yet.\n";
            return;
        }

        cout << '\n' << left << setw(6) << "ID"
             << setw(20) << "Description"
             << setw(16) << "Category"
             << setw(12) << "Amount" << "Date\n";

        cout << string(70, '-') << '\n';

        for (const auto& e : expenses)
            e.display();
    }

    void categoryReport() const {
        vector<string> categories;
        vector<double> totals;

        for (const auto& e : expenses) {
            string category = e.getCategory();
            int index = 0;

            while (index < (int)categories.size() &&
                   categories[index] != category)
                index++;

            if (index == (int)categories.size()) {
                categories.push_back(category);
                totals.push_back(0);
            }

            totals[index] += e.getAmount();
        }

        cout << "\n--- Category-wise Report ---\n";
        for (int i = 0; i < (int)categories.size(); i++)
            cout << categories[i] << ": Rs. "
                 << fixed << setprecision(2)
                 << totals[i] << '\n';
    }

    void report() const {
        cout << "\n--- Expense Report ---\n";
        cout << "Number of expenses: " << expenses.size() << '\n';
        cout << "Total recorded spending: Rs. "
             << fixed << setprecision(2) << total() << '\n';

        if (budget.getLimit() > 0) {
            cout << "Budget: Rs. " << budget.getLimit() << '\n';
            budget.check(total());
        }

        categoryReport();
    }
};

int main() {
    ExpenseManager manager;
    int choice;

    do {
        cout << "\n===== SMART EXPENSE & BUDGET MANAGER =====\n"
             << "1. Set Budget\n"
             << "2. Add Expense\n"
             << "3. View All Expenses\n"
             << "4. View Total Spending\n"
             << "5. Category-wise Report\n"
             << "6. Generate Full Report\n"
             << "7. Exit\n"
             << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a number.\n";
            continue;
        }

        try {
            switch (choice) {
            case 1:
                manager.setBudget();
                break;
            case 2:
                manager.addExpense();
                break;
            case 3:
                manager.viewExpenses();
                break;
            case 4:
                cout << "Total spending: Rs. "
                     << fixed << setprecision(2)
                     << manager.total() << '\n';
                break;
            case 5:
                manager.categoryReport();
                break;
            case 6:
                manager.report();
                break;
            case 7:
                cout << "Thank you for using the Expense Manager!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << '\n';
        }
    } while (choice != 7);

    return 0;
}
