// Copyright 2017 Nest Labs, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <map>
#include <string>
#include <iostream>
#include <regex>

#include "graph.hpp"
#include "detector.hpp"
#include "sharedptr.hpp"
#include "processorcontainer.hpp"

using namespace DetectorGraph;
using std::cout;
using std::endl;

/**
 * @file portuguesetranslator.cpp
 * @brief A dictionary-based translator that uses shared memory in TopicStates.
 *
 * @section ex-pt-intro Introduction
 * This examples implements a very basic translator based only on word
 * replacements.
 *
 * @section ex-pt-sharing-mem Using Shared Memory in TopicStates
 * DetectorGraph uses C++ copies to propagate information across Topics &
 * Detectors. This is fine & desirable for small amounts of data but can become
 * prohibitive for large objects.
 * In such cases the recommended pattern is to wrap the large resource using
 * an appropriate smart pointer (e.g. std::shared_ptr).
 *
 * The one caveat is that, as Detectors start accessing shared memory, the
 * DetectorGraph framework alone cannot guarantee the hermetic nature of
 * detectors - a detector's data (i.e. the data a member pointer points to) may
 * change even though no Evaluate('new value') for that member was called.
 *
 * In cases where the large data piece only has only read-only use, a
 * const_shared_ptr can be used to partially address the concern mentioned
 * above.
 *
 * In this example, the `TranslationDictionary` TopicState carries a large,
 * immutable object and does so using a shared_ptr<const T>:
 @snippetlineno portuguesetranslator.cpp Immutable Shared Memory TopicState
 *
 * A different example that also uses this pattern is
 * [Fancy Vending Machine](@ref fancyvendingmachine.cpp):
 @snippetlineno fancyvendingmachine.cpp Immutable Shared Memory TopicState
 *
 * @section ex-pt-arch Architecture
 * This sample implements the following graph:
 *
 *  @dot "TextTranslatorGraph"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    size="12,5";
    "TranslationDictionary" [label="0:TranslationDictionary",style=filled, shape=box, color=lightblue];
    "EnglishText" [label="1:EnglishText",style=filled, shape=box, color=lightblue];
        "TranslationDictionary" -> "EnglishToPortugueseTranslator";
        "EnglishText" -> "EnglishToPortugueseTranslator";
        "EnglishToPortugueseTranslator" -> "PortugueseText";
    "PortugueseText" [label="3:PortugueseText",style=filled, shape=box, color=limegreen];
    "EnglishToPortugueseTranslator" [label="2:EnglishToPortugueseTranslator", color=blue];
}
 *  @enddot
 *
 */
/// @cond DO_NOT_DOCUMENT
/**
 * @brief A TopicState implementation that wraps a shared resource.
 *
 * This TopicState wraps a - potentially massive - std::map. Its Topic
 * can then be subscribed by any number of Detectors - and those detectors
 * can keep a copy of this TopicState around - with no impact on memory/ram
 * usage; the copies are only shallow copies.
 */

using Text2TextMap = std::map<std::string, std::string>;

//! [Immutable Shared Memory TopicState]
struct TranslationDictionary : public TopicState
{
    TranslationDictionary()
    : map(std::make_shared<Text2TextMap>())
    {
    }

    TranslationDictionary(const std::shared_ptr<const Text2TextMap>& aMapPtr)
    : map(aMapPtr)
    {
    }

    bool Lookup(std::string inStr, std::string& outStr) const
    {
        Text2TextMap::const_iterator lookupIterator = map->find(inStr);
        if (lookupIterator != map->end())
        {
            outStr = lookupIterator->second;
            return true;
        }
        else
        {
            return false;
        }
    };

    std::shared_ptr<const Text2TextMap> map;
};
//! [Immutable Shared Memory TopicState]

struct EnglishText : public TopicState
{
    EnglishText(std::string aText = "") : text(aText) {}
    std::string text;
};

struct PortugueseText : public TopicState
{
    PortugueseText(std::string aText = "") : text(aText) {}
    std::string text;
};

/**
 * This Detector keeps a shallow local copy of a potentially large object -
 * TranslationDictionary - and uses it whenever EnglishText changes to produce
 * a PortugueseText.
 */
class EnglishToPortugueseTranslator :
public Detector,
public SubscriberInterface<TranslationDictionary>,
public SubscriberInterface<EnglishText>,
public Publisher<PortugueseText>
{
public:
    EnglishToPortugueseTranslator(Graph* graph) : Detector(graph)
    {
        Subscribe<TranslationDictionary>(this);
        Subscribe<EnglishText>(this);
        SetupPublishing<PortugueseText>(this);
    }

    // Caches
    TranslationDictionary mDatabase;

    virtual void Evaluate(const TranslationDictionary& aDatabase)
    {
        mDatabase = aDatabase;
    }

    virtual void Evaluate(const EnglishText& aEnglishText)
    {
        std::vector<std::string> outputWords;
        auto englishWords = split(aEnglishText.text, "\\s+");
        for (auto inWord : englishWords)
        {
            std::string outWord;
            bool hasTranslation = mDatabase.Lookup(inWord, outWord);
            outputWords.push_back((hasTranslation) ? outWord : inWord);
        }

        Publish(PortugueseText(join(outputWords, " ")));
    }

private:
    std::vector<std::string> split(const std::string& input, const std::string& regex) {
        std::regex re(regex);
        // passing -1 as the submatch index parameter performs splitting
        std::sregex_token_iterator first{input.begin(), input.end(), re, -1 /* ha-ha-ha */};
        std::sregex_token_iterator last; // ha-ha
        return {first, last};
    }
    std::string join(const std::vector<std::string>& lst, const std::string& delim)
    {
        std::string ret;
        for(const auto &s : lst) {
            if(!ret.empty())
                ret += delim;
            ret += s;
        }
        return ret;
    }
};

/**
 * This class is a container for Graph+Detectors+Topics and it provides a joint
 * method for performing graph evaluations and *using* its output (printing in
 * this case).
 */
class TextTranslatorGraph : public ProcessorContainer
{
public:
    TextTranslatorGraph() : mEnglishToPortugueseTranslator(&mGraph)
    {
    }

    virtual void ProcessOutput()
    {
        auto actionRequestTopic = mGraph.ResolveTopic<PortugueseText>();
        if (actionRequestTopic->HasNewValue())
        {
            const PortugueseText& actionRequest = actionRequestTopic->GetNewValue();
            cout << actionRequest.text << endl;
        }
    }

private:
    EnglishToPortugueseTranslator mEnglishToPortugueseTranslator;
};

int main()
{
    TextTranslatorGraph graph;

    std::shared_ptr<Text2TextMap> database = std::make_shared<Text2TextMap>();
    database->insert({"brown", "marrom"});
    database->insert({"dog", "cao"});
    database->insert({"fox", "raposao"});
    database->insert({"jumped", "pulou"});
    database->insert({"lazy", "preguicoso"});
    database->insert({"over", "sobre"});
    database->insert({"quick", "rapido"});
    database->insert({"the", "o"});
    // Let's imagine this is a massive list.

    graph.ProcessData(TranslationDictionary(database));

    graph.ProcessData(EnglishText("the quick brown fox jumped over the lazy dog"));
}

/// @endcond DO_NOT_DOCUMENT
