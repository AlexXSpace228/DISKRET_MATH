#include <iostream>
#include <new>
#include <cstddef>
#include <cerrno>
#include <climits>
#include <cctype>
#include <cstdlib>
#include <random>
#include <windows.h>

class IntegerSet {
private:
	int* elements;
	std::size_t count;
	std::size_t capacity;

	void EnsureCapacity(std::size_t required) {
		if (required <= capacity) {
			return;
		}

		std::size_t newCapacity = capacity == 0 ? 4 : capacity;
		while (newCapacity < required) {
			newCapacity *= 2;
		}

		int* newElements = new int[newCapacity];
		for (std::size_t i = 0; i < count; ++i) {
			newElements[i] = elements[i];
		}
		delete[] elements;
		elements = newElements;
		capacity = newCapacity;
	}

public:
	IntegerSet() : elements(nullptr), count(0), capacity(0) {}

	~IntegerSet() {
		delete[] elements;
	}

	IntegerSet(const IntegerSet&) = delete;
	IntegerSet& operator=(const IntegerSet&) = delete;

	IntegerSet(IntegerSet&& other) noexcept
		: elements(other.elements), count(other.count), capacity(other.capacity) {
		other.elements = nullptr;
		other.count = 0;
		other.capacity = 0;
	}

	IntegerSet& operator=(IntegerSet&& other) noexcept {
		if (this != &other) {
			delete[] elements;
			elements = other.elements;
			count = other.count;
			capacity = other.capacity;
			other.elements = nullptr;
			other.count = 0;
			other.capacity = 0;
		}
		return *this;
	}

	std::size_t Size() const {
		return count;
	}

	int At(std::size_t index) const {
		return elements[index];
	}

	bool Contains(int value) const {
		for (std::size_t i = 0; i < count; ++i) {
			if (elements[i] == value) {
				return true;
			}
		}
		return false;
	}

	bool Add(int value) {
		if (Contains(value)) {
			return false;
		}
		EnsureCapacity(count + 1);
		elements[count++] = value;
		return true;
	}

	void Clear() {
		count = 0;
	}
};

class SetCalculator {
public:
	static IntegerSet Union(const IntegerSet& first, const IntegerSet& second) {
		IntegerSet result;
		for (std::size_t i = 0; i < first.Size(); ++i) {
			result.Add(first.At(i));
		}
		for (std::size_t i = 0; i < second.Size(); ++i) {
			result.Add(second.At(i));
		}
		return result;
	}

	static IntegerSet Intersection(const IntegerSet& first, const IntegerSet& second) {
		IntegerSet result;
		for (std::size_t i = 0; i < first.Size(); ++i) {
			if (second.Contains(first.At(i))) {
				result.Add(first.At(i));
			}
		}
		return result;
	}

	static IntegerSet Difference(const IntegerSet& first, const IntegerSet& second) {
		IntegerSet result;
		for (std::size_t i = 0; i < first.Size(); ++i) {
			if (!second.Contains(first.At(i))) {
				result.Add(first.At(i));
			}
		}
		return result;
	}

	static IntegerSet SymmetricDifference(const IntegerSet& first, const IntegerSet& second) {
		IntegerSet result = Difference(first, second);
		for (std::size_t i = 0; i < second.Size(); ++i) {
			if (!first.Contains(second.At(i))) {
				result.Add(second.At(i));
			}
		}
		return result;
	}

	static IntegerSet Complement(const IntegerSet& set, const IntegerSet& universal) {
		return Difference(universal, set);
	}

	static bool IsSubset(const IntegerSet& subset, const IntegerSet& superset) {
		for (std::size_t i = 0; i < subset.Size(); ++i) {
			if (!superset.Contains(subset.At(i))) {
				return false;
			}
		}
		return true;
	}
};

const int MaxInputCount = 100000;

bool ReadInteger(const char* prompt, int minimum, int maximum, int& value) {
	char line[256];
	while (true) {
		std::cout << prompt;
		if (!std::cin.getline(line, sizeof(line))) {
			return false;
		}

		char* end = nullptr;
		errno = 0;
		const long parsed = std::strtol(line, &end, 10);
		while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end))) {
			++end;
		}
		if (end == line || *end != '\0' || errno == ERANGE ||
			parsed < minimum || parsed > maximum) {
			std::cout << "Ошибка: введите целое число от " << minimum << " до " << maximum << ".\n";
			continue;
		}

		value = static_cast<int>(parsed);
		return true;
	}
}

void PrintSet(const char* name, const IntegerSet& set) {
	std::cout << name << " = {";
	for (std::size_t i = 0; i < set.Size(); ++i) {
		if (i != 0) {
			std::cout << ", ";
		}
		std::cout << set.At(i);
	}
	std::cout << "}\n";
}

bool ReadSetElements(IntegerSet& set, const IntegerSet* universal) {
	int count = 0;
	if (!ReadInteger("Количество элементов: ", 0, MaxInputCount, count)) {
		return false;
	}

	set.Clear();
	for (int i = 0; i < count; ++i) {
		int value = 0;
		while (true) {
			if (!ReadInteger("Элемент: ", INT_MIN, INT_MAX, value)) {
				return false;
			}
			if (universal != nullptr && !universal->Contains(value)) {
				std::cout << "Ошибка: элемент не принадлежит U. Повторите ввод.\n";
				continue;
			}
			if (!set.Add(value)) {
				std::cout << "Элемент уже есть в множестве; повтор не добавлен.\n";
			}
			break;
		}
	}
	return true;
}

bool GenerateUniversal(IntegerSet& universal, std::mt19937& generator) {
	int minimum = 0;
	int maximum = 0;
	int count = 0;
	if (!ReadInteger("Нижняя граница диапазона: ", INT_MIN, INT_MAX, minimum) ||
		!ReadInteger("Верхняя граница диапазона: ", minimum, INT_MAX, maximum)) {
		return false;
	}

	const long long rangeSize = static_cast<long long>(maximum) - minimum + 1;
	while (true) {
		if (!ReadInteger("Количество уникальных элементов U: ", 0, MaxInputCount, count)) {
			return false;
		}
		if (count <= rangeSize) {
			break;
		}
		std::cout << "Ошибка: количество элементов превышает размер диапазона.\n";
	}

	std::uniform_int_distribution<long long> distribution(minimum, maximum);
	IntegerSet generated;
	while (generated.Size() < static_cast<std::size_t>(count)) {
		generated.Add(static_cast<int>(distribution(generator)));
	}
	universal = static_cast<IntegerSet&&>(generated);
	return true;
}

bool GenerateSubset(IntegerSet& set, const IntegerSet& universal, std::mt19937& generator) {
	int count = 0;
	if (!ReadInteger("Количество элементов подмножества: ", 0,
		static_cast<int>(universal.Size()), count)) {
		return false;
	}

	set.Clear();
	if (count == 0) {
		return true;
	}
	std::uniform_int_distribution<std::size_t> distribution(0, universal.Size() - 1);
	while (set.Size() < static_cast<std::size_t>(count)) {
		set.Add(universal.At(distribution(generator)));
	}
	return true;
}

bool ConfigureSet(IntegerSet& set, const IntegerSet* universal, std::mt19937& generator) {
	int method = 0;
	if (!ReadInteger("1 — ввод с клавиатуры, 2 — случайная генерация: ", 1, 2, method)) {
		return false;
	}
	if (method == 1) {
		return ReadSetElements(set, universal);
	}
	if (universal == nullptr) {
		return GenerateUniversal(set, generator);
	}
	return GenerateSubset(set, *universal, generator);
}

bool ReadSetIndex(const char* prompt, int& index) {
	return ReadInteger(prompt, 1, 5, index);
}

int main() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	const char* names[5] = { "A", "B", "C", "D", "E" };
	IntegerSet universal;
	IntegerSet sets[5];
	std::mt19937 generator(std::random_device{}());
	bool universalConfigured = false;
	int choice = -1;

	while (choice != 0) {
		std::cout << "\n=== Калькулятор множеств ===\n"
			<< "1. Задать универсальное множество U\n"
			<< "2. Задать множество A-E\n"
			<< "3. Показать все множества\n"
			<< "4. Операция над двумя множествами\n"
			<< "5. Дополнение множества до U\n"
			<< "6. Проверить принадлежность элемента\n"
			<< "7. Проверить включение подмножеств\n"
			<< "0. Выход\n";
		if (!ReadInteger("Выберите действие: ", 0, 7, choice)) {
			break;
		}

		if (choice == 1) {
			if (!ConfigureSet(universal, nullptr, generator)) {
				break;
			}
			universalConfigured = true;
			for (int i = 0; i < 5; ++i) {
				sets[i].Clear();
			}
			std::cout << "U задано. Множества A-E очищены.\n";
		}
		else if (choice == 2) {
			if (!universalConfigured) {
				std::cout << "Сначала задайте U.\n";
				continue;
			}
			int index = 0;
			if (!ReadSetIndex("Номер множества (1-A, 2-B, 3-C, 4-D, 5-E): ", index) ||
				!ConfigureSet(sets[index - 1], &universal, generator)) {
				break;
			}
			PrintSet(names[index - 1], sets[index - 1]);
		}
		else if (choice == 3) {
			if (!universalConfigured) {
				std::cout << "Сначала задайте U.\n";
				continue;
			}
			PrintSet("U", universal);
			for (int i = 0; i < 5; ++i) {
				PrintSet(names[i], sets[i]);
			}
		}
		else if (choice == 4) {
			if (!universalConfigured) {
				std::cout << "Сначала задайте U и множества.\n";
				continue;
			}
			int operation = 0;
			int firstIndex = 0;
			int secondIndex = 0;
			std::cout << "1 — пересечение, 2 — объединение, 3 — разность, 4 — симметрическая разность\n";
			if (!ReadInteger("Операция: ", 1, 4, operation) ||
				!ReadSetIndex("Первое множество (1-A ... 5-E): ", firstIndex) ||
				!ReadSetIndex("Второе множество (1-A ... 5-E): ", secondIndex)) {
				break;
			}
			IntegerSet result;
			const IntegerSet& first = sets[firstIndex - 1];
			const IntegerSet& second = sets[secondIndex - 1];
			if (operation == 1) {
				result = SetCalculator::Intersection(first, second);
			}
			else if (operation == 2) {
				result = SetCalculator::Union(first, second);
			}
			else if (operation == 3) {
				result = SetCalculator::Difference(first, second);
			}
			else {
				result = SetCalculator::SymmetricDifference(first, second);
			}
			PrintSet("Результат", result);
		}
		else if (choice == 5) {
			if (!universalConfigured) {
				std::cout << "Сначала задайте U.\n";
				continue;
			}
			int index = 0;
			if (!ReadSetIndex("Дополнить какое множество (1-A ... 5-E): ", index)) {
				break;
			}
			IntegerSet result = SetCalculator::Complement(sets[index - 1], universal);
			PrintSet("Дополнение", result);
		}
		else if (choice == 6) {
			if (!universalConfigured) {
				std::cout << "Сначала задайте U.\n";
				continue;
			}
			int index = 0;
			int value = 0;
			if (!ReadSetIndex("Проверить множество (1-A ... 5-E): ", index) ||
				!ReadInteger("Введите целый элемент: ", INT_MIN, INT_MAX, value)) {
				break;
			}
			std::cout << (sets[index - 1].Contains(value) ? "Элемент принадлежит множеству.\n" :
				"Элемент не принадлежит множеству.\n");
		}
		else if (choice == 7) {
			if (!universalConfigured) {
				std::cout << "Сначала задайте U.\n";
				continue;
			}
			int subsetIndex = 0;
			int supersetIndex = 0;
			if (!ReadSetIndex("Проверяемое подмножество (1-A ... 5-E): ", subsetIndex) ||
				!ReadSetIndex("Множество, в которое проверяем включение (1-A ... 5-E): ", supersetIndex)) {
				break;
			}
			if (SetCalculator::IsSubset(sets[subsetIndex - 1], sets[supersetIndex - 1])) {
				std::cout << names[subsetIndex - 1] << " является подмножеством "
					<< names[supersetIndex - 1] << ".\n";
			}
			else {
				std::cout << names[subsetIndex - 1] << " не является подмножеством "
					<< names[supersetIndex - 1] << ".\n";
			}
		}
	}

	return 0;
}