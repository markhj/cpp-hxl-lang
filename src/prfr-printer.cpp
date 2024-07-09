#include "hxl-lang/utilities/prfr-printer.h"
#include <iostream>

/**
 * The fixed width of each cell in the table.
 */
uint8_t cellSize = 24;

/**
 * Helper function to draw the dashed separator line.
 *
 * Simply add ``c`` to a string until it has the desired size.
 *
 * @param c
 * @param size
 */
inline void separatorLine(char c, uint8_t size) {
    std::string s;
    while (s.length() < size) {
        s += c;
    }
    std::cout << s << "\n";
}

/**
 * Draw a row of cells.
 * Space on a per-cell basis not taken up by text will be filled by ``fill``.
 * Cell dividers are defined by ``separator``.
 *
 * @param cells
 * @param fill
 * @param separator
 * @param blockSize
 */
inline void cells(const std::vector<std::string> &cells,
                  char fill,
                  char separator,
                  uint8_t blockSize) {
    std::string cell;
    size_t current = 0;
    uint8_t cellPos = 0;
    std::string cellPad = std::string() + separator + " ";

    // The idea is that we create a loop that just runs non-stop
    // through the calculated total width of the row, instead of
    // iterating over each cell.
    for (int i = 0; i < cells.size() * cellSize; ++i) {
        if (i == 0) {
            cell += cellPad;
        }

        // If the current position within the cell is still within
        // bounds, and there's text remaining in the current cell,
        // it will be printed. Otherwise, we fill whitespace.
        if (cellPos < cellSize - cellPad.length()) {
            cell += cells[current].length() >= cellPos ? cells[current][cellPos] : fill;
        }

        ++cellPos;

        // When we hit the exact spot between two cells, we increase
        // the cell (text) we're looking at, and add divider/separator.
        if (i > 1 && i % blockSize == 0) {
            ++current;
            cellPos = 0;
            cell += cellPad;
        }
    }

    std::cout << cell << "\n";
}

/**
 * Draw a row with a title and performance metrics.
 *
 * @param title
 * @param time
 */
inline void row(const std::string &title,
                const std::optional<HXL::ExecutionTime> &time) {
    std::string milli, micro;

    // Text to show if there's no metric for this category.
    milli = micro = "-";

    if (time.has_value()) {
        HXL::PerformanceTime ms = time.value().ms;
        micro = std::to_string(ms);
        milli = std::to_string(ms / 1000);
    }

    // Draw the cells
    cells({title, milli + " ms", micro}, ' ', '|', cellSize);
}

void HXL::Utilities::PerformanceResultsPrinter::print(const HXL::PerformanceResults &results) {
    // Header
    separatorLine('-', cellSize * 3);
    cells({"Metric", "Millisecs.", "Microsecs."}, ' ', '|', cellSize);
    separatorLine('-', cellSize * 3);

    // Rows with each metric
    row("Tokenization", results.tokenization);
    row("Parsing", results.parsing);
    row("Semantic analysis", results.semanticAnalysis);
    row("Transformation", results.transformer);
    row("Schema validation", results.schemaValidation);
    row("Deserialization", results.deserialization);

    // Total (footer)
    separatorLine('-', cellSize * 3);
    row("Total", results.getTotal());
    separatorLine('-', cellSize * 3);

    std::cout.flush();
}
