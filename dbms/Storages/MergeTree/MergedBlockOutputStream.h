#pragma once

#include <Storages/MergeTree/IMergedBlockOutputStream.h>
#include <Columns/ColumnArray.h>


namespace DB
{

/** To write one part.
  * The data refers to one partition, and is written in one part.
  */
class MergedBlockOutputStream final : public IMergedBlockOutputStream
{
public:
    MergedBlockOutputStream(
        const MergeTreeDataPartPtr & data_part,
        const NamesAndTypesList & columns_list_,
        CompressionCodecPtr default_codec,
        bool blocks_are_granules_size = false);

    MergedBlockOutputStream(
        const MergeTreeDataPartPtr & data_part,
        const NamesAndTypesList & columns_list_,
        CompressionCodecPtr default_codec,
        const MergeTreeData::DataPart::ColumnToSize & merged_column_to_size,
        size_t aio_threshold,
        bool blocks_are_granules_size = false);

    Block getHeader() const override { return storage.getSampleBlock(); }

    /// If the data is pre-sorted.
    void write(const Block & block) override;

    /** If the data is not sorted, but we have previously calculated the permutation, that will sort it.
      * This method is used to save RAM, since you do not need to keep two blocks at once - the original one and the sorted one.
      */
    void writeWithPermutation(const Block & block, const IColumn::Permutation * permutation);

    void writeSuffix() override;

    /// Finilize writing part and fill inner structures
    void writeSuffixAndFinalizePart(
            MergeTreeData::MutableDataPartPtr & new_part,
            const NamesAndTypesList * total_columns_list = nullptr,
            MergeTreeData::DataPart::Checksums * additional_column_checksums = nullptr);

private:
    /** If `permutation` is given, it rearranges the values in the columns when writing.
      * This is necessary to not keep the whole block in the RAM to sort it.
      */
    void writeImpl(const Block & block, const IColumn::Permutation * permutation);

private:
    NamesAndTypesList columns_list;

    size_t rows_count = 0;
};

}